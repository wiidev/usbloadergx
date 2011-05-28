/*-------------------------------------------------------------

di2.h -- Drive Interface library

Written by rodries
Modified from (and supplemental to) original libdi library:

Team Twiizers
Copyright (C) 2008

Erant
marcan


This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.

-------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ogc/ipc.h>
#include <ogc/ios.h>
#include <ogc/mutex.h>
#include <errno.h>

#include <unistd.h>
#include <malloc.h>
#include <ogc/lwp.h>
#include <ogc/lwp_watchdog.h>

#include "di2.h"

extern int di_fd;

int _DI2_ReadDVD_ReadID(void* buf, uint32_t len, uint32_t lba);
int _DI2_ReadDVD_ReadID_Async(void* buf, uint32_t len, uint32_t lba, ipccallback ipc_cb);

int _DI_ReadDVD_A8(void* buf, uint32_t len, uint32_t lba);
int _DI_ReadDVD_D0(void* buf, uint32_t len, uint32_t lba);

int _DI_ReadDVD_A8_Async(void* buf, uint32_t len, uint32_t lba, ipccallback ipc_cb);
int _DI_ReadDVD_D0_Async(void* buf, uint32_t len, uint32_t lba, ipccallback ipc_cb);

void _DI2_SetCallback(int di_command, ipccallback);
static int _cover_callback(int ret, void* usrdata);

int state = DVD_INIT | DVD_NO_DISC;

static mutex_t bufferMutex = LWP_MUTEX_NULL;
static uint32_t outbuf[8] __attribute__((aligned(32)));
static uint32_t dic[8] __attribute__((aligned(32)));
static char di_path[] ATTRIBUTE_ALIGN(32) = "/dev/di";

read_func DI2_ReadDVDptr = NULL;
read_func_async DI2_ReadDVDAsyncptr = NULL;
di_callback di_cb = NULL;

#define DEFAULT_TIME_STOP_MOTOR 60  // 60 secs
#define MIN_TIME_STOP_MOTOR 8  // the min value accepted
static bool motorthreadexit = false;
static lwp_t motorthread = LWP_THREAD_NULL;
static bool motor_stopped = true;
static u64 LastAccess = 0;
static unsigned int TimeStopMotor = DEFAULT_TIME_STOP_MOTOR;

int _DI2_ReadDVD_Check(void* buf, uint32_t len, uint32_t lba);

void DI2_SetDVDMotorStopSecs(int secs) //in seconds
{

    if (secs < MIN_TIME_STOP_MOTOR)
        return ;

    LastAccess = ticks_to_secs(gettime());

    TimeStopMotor = secs;
}

unsigned int DI2_GetDVDMotorStopSecs() //in seconds
{
    return TimeStopMotor;
}

static bool DVD_DiscPresent()
{
    uint32_t val;

    DI2_GetCoverRegister(&val);

    if (val & 0x2)
        return true;

    return false;
}

static void * motorthreadfunc(void *arg)
{
    long sleeptime;
    bool first = true;

    while (1)
    {
        if (motor_stopped)
        {
            first = true;
        }

        else
        {
            if (first) // change state from stop to start
            {
                sleeptime = 10 * 1000 * 1000; // 10 sec

                while (sleeptime > 0)
                {
                    if (motorthreadexit)
                        return NULL;

                    usleep(100);

                    sleeptime -= 100;
                }

                LastAccess = ticks_to_secs(gettime());
                first = false;
            }
            if ((ticks_to_secs(gettime()) - LastAccess) > TimeStopMotor)
            { // we have to stop motor

                if (DVD_DiscPresent()) // only stop if dvd is present
                    DI2_StopMotor();
            }
        }

        sleeptime = 1000 * 1000; // 1 sec

        while (sleeptime > 0)
        {
            if (motorthreadexit)
                return NULL;

            usleep(100);

            sleeptime -= 100;
        }
    }

    return NULL;
}

void DI2_StartMotorThread()
{
    if (motorthread == LWP_THREAD_NULL)
        LWP_CreateThread(&motorthread, motorthreadfunc, NULL, NULL, 0, 63); // create thread
}

void DI2_StopMotorThread()
{
    motorthreadexit = true; // signal thread to close

    if (motorthread != LWP_THREAD_NULL)
        LWP_JoinThread(motorthread, NULL); // wait for thread to finish

    motorthread = LWP_THREAD_NULL;

    motorthreadexit = false;

    motor_stopped = true;
}

int DI2_StartMotor()
{
    if (state == (DVD_INIT | DVD_NO_DISC) || (DI2_ReadDVDptr == _DI2_ReadDVD_Check))
        return 0;

    if (motor_stopped && motorthreadexit == false)
    {
        DI2_Mount();
        unsigned int t1, t2;
        t1 = ticks_to_secs(gettime());

        while (DI2_GetStatus() & DVD_INIT)
        {
            t2 = ticks_to_secs(gettime());

            if (t2 - t1 > 15) return -1;

            usleep(5000);
        }

        motor_stopped = false;
    }

    return 0;
}

void CheckAccess()
{
    DI2_StartMotor();
    LastAccess = ticks_to_secs(gettime());
}

///// Cache
#define CACHE_FREE 0xFFFFFFFF
#define BLOCK_SIZE 0x800
#define CACHEBLOCKS 26

typedef struct
{
    uint32_t block;
    void *ptr;
}

cache_page;
static cache_page *cache_read = NULL;

static void CreateDVDCache()
{
    if (cache_read != NULL)
        return ;

    cache_read = (cache_page *) malloc(sizeof(cache_page));

    if (cache_read == NULL)
        return ;

    cache_read->block = CACHE_FREE;

    cache_read->ptr = memalign(32, BLOCK_SIZE * CACHEBLOCKS);

    if (cache_read->ptr == NULL)
    {
        if (cache_read->ptr)
            free(cache_read->ptr);

        free(cache_read);

        cache_read = NULL;

        return ;
    }

    memset(cache_read->ptr, 0, BLOCK_SIZE);
}

static int ReadBlockFromCache(void *buf, uint32_t len, uint32_t block)
{
    int retval;

    if (cache_read == NULL)
        return DI2_ReadDVDptr(buf, len, block);

    if ((block >= cache_read->block) && (block + len < (cache_read->block + CACHEBLOCKS)))
    {
        memcpy(buf, cache_read->ptr + ((block - cache_read->block) * BLOCK_SIZE), BLOCK_SIZE * len);
        return 0;
    }

    if (len > CACHEBLOCKS)
        return DI2_ReadDVDptr(buf, len, block);

    retval = DI2_ReadDVDptr(cache_read->ptr, CACHEBLOCKS, block);

    if (retval)
    {
        cache_read->block = CACHE_FREE;
        return retval;
    }

    cache_read->block = block;
    memcpy(buf, cache_read->ptr, len * BLOCK_SIZE);

    return 0;
}

/*
 Initialize the DI interface, should always be called first!
 */

s32 __DI_LoadStub();

int DI2_Init(bool dvdx)
{
    static int dvdxinit = 0;

    state = DVD_INIT | DVD_NO_DISC;

    if (dvdx && !dvdxinit)
    {
        __DI_LoadStub(); // Marcan's 1337 magics happen here!
        dvdxinit = 1;
    }

    if (di_fd < 0)
        di_fd = IOS_Open(di_path, 2);

    if (bufferMutex == LWP_MUTEX_NULL)
        LWP_MutexInit(&bufferMutex, false);

    LastAccess = 0;

    CreateDVDCache();

    return (di_fd >= 0) ? di_fd : -1;
}

void DI2_Mount()
{
    uint32_t status;

    if (DI2_GetCoverRegister(&status) != 0)
    {
        state = DVD_NO_DISC;
        return ;
    }

    if ((status & DVD_COVER_DISC_INSERTED) == 0)
    {
        state = DVD_NO_DISC;
        return ;
    }

    state = DVD_INIT | DVD_NO_DISC;
    _cover_callback(1, NULL); // Initialize the callback chain
    LastAccess = ticks_to_secs(gettime());
    DI2_StopMotorThread();

    if (cache_read != NULL)
        cache_read->block = CACHE_FREE; // reset cache

    LastAccess = ticks_to_secs(gettime());

    DI2_StartMotorThread();
}

void DI2_Close()
{
    DI2_StopMotorThread();
    LastAccess = 0;

    if (di_fd > 0)
    {
        IOS_Close(di_fd);
    }

    di_fd = -1;

    DI2_ReadDVDptr = NULL;
    DI2_ReadDVDAsyncptr = NULL;
    state = DVD_INIT | DVD_NO_DISC;

    if (bufferMutex)
    {
        LWP_MutexDestroy(bufferMutex);
        bufferMutex = 0;
    }
}

#define COVER_CLOSED (*((uint32_t*)usrdata) & DVD_COVER_DISC_INSERTED)

int _DI2_ReadDVD_Check(void* buf, uint32_t len, uint32_t lba)
{
    int ret;

    ret = _DI_ReadDVD_D0(buf, len, lba);

    if (ret >= 0)
    {
        state = state | DVD_D0;
        DI2_ReadDVDptr = _DI_ReadDVD_D0;
        DI2_ReadDVDAsyncptr = _DI_ReadDVD_D0_Async;
        motor_stopped = false;
        //printf("libdi: D0 functions detected\n");
        return ret;
    }

    ret = _DI_ReadDVD_A8(buf, len, lba);

    if (ret >= 0)
    {
        state = state | DVD_A8;
        DI2_ReadDVDptr = _DI_ReadDVD_A8;
        DI2_ReadDVDAsyncptr = _DI_ReadDVD_A8_Async;
        motor_stopped = false;
        //printf("libdi: A8 functions detected\n");
        return ret;
    }

    //printf("libdi: error detection D0/A8 functions\n");
    return ret;
}

int _DI2_ReadDVD_Check_Async(void* buf, uint32_t len, uint32_t lba, ipccallback ipc_cb)
{ // is bad code has to be done correctly using callback func
    int ret;

    ret = _DI_ReadDVD_D0_Async(buf, len, lba, ipc_cb);

    if (ret >= 0)
    {
        state = state | DVD_D0;
        DI2_ReadDVDptr = _DI_ReadDVD_D0;
        DI2_ReadDVDAsyncptr = _DI_ReadDVD_D0_Async;
        motor_stopped = false;
        return ret;
    }

    ret = _DI_ReadDVD_A8_Async(buf, len, lba, ipc_cb);

    if (ret >= 0)
    {
        state = state | DVD_A8;
        DI2_ReadDVDptr = _DI_ReadDVD_A8;
        DI2_ReadDVDAsyncptr = _DI_ReadDVD_A8_Async;
        motor_stopped = false;
        return ret;
    }

    return ret;
}

static int _cover_callback(int ret, void* usrdata)
{
    static int cur_state = 0;
    static int retry_count = LIBDI_MAX_RETRIES;
    const int callback_table[] =
        {
            DVD_GETCOVER, DVD_WAITFORCOVERCLOSE, DVD_RESET, DVD_IDENTIFY,  // This one will complete when the drive is ready.
            DVD_READ_DISCID, 0
        };

    const int return_table[] =
                        { 1, 1, 4, 1, 1 };

    if (cur_state > 1)
        state &= ~DVD_NO_DISC;

    if (callback_table[cur_state])
    {
        if (ret == return_table[cur_state])
        {
            if (cur_state == 1 && COVER_CLOSED) // Disc inside, skipping wait for cover.
                cur_state += 2;
            else
                cur_state++; // If the previous callback succeeded, moving on to the next

            retry_count = LIBDI_MAX_RETRIES;
        }

        else
        {
            retry_count--;

            if (retry_count < 0)
            { // Drive init failed for unknown reasons.
                retry_count = LIBDI_MAX_RETRIES;
                cur_state = 0;
                state = DVD_UNKNOWN;
                return 0;
            }
        }

        _DI2_SetCallback(callback_table[cur_state - 1], _cover_callback);

    }

    else // Callback chain has completed OK. The drive is ready.
    {
        state = DVD_READY;

        retry_count = 1;
        state = DVD_READY;

        DI2_ReadDVDptr = _DI2_ReadDVD_Check;
        DI2_ReadDVDAsyncptr = _DI2_ReadDVD_Check_Async;

        if (di_cb)
            di_cb(state, 0);

        retry_count = LIBDI_MAX_RETRIES;

        cur_state = 0;

    }

    return 0;
}

/* Get current status, will return the API status */
int DI2_GetStatus()
{
    return state;
}

void DI2_SetInitCallback(di_callback cb)
{
    di_cb = cb;
}

void _DI2_SetCallback(int ioctl_nr, ipccallback ipc_cb)
{
    if (!ipc_cb)
        return ;

    LWP_MutexLock(bufferMutex);

    memset(dic, 0x00, sizeof(dic));

    dic[0] = ioctl_nr << 24;

    dic[1] = (ioctl_nr == DVD_RESET) ? 1 : 0; // For reset callback. Dirty, I know...

    IOS_IoctlAsync(di_fd, ioctl_nr, dic, 0x20, outbuf, 0x20, ipc_cb, outbuf);

    //We're done with the buffer now.
    LWP_MutexUnlock(bufferMutex);
}

/*
 Request an identification from the drive, returned in a DI_DriveID struct
 */
int DI2_Identify(DI_DriveID* id)
{
    if (!id)
    {
        errno = EINVAL;
        return -1;
    }

    CheckAccess();
    LWP_MutexLock(bufferMutex);
    dic[0] = DVD_IDENTIFY << 24;

    int ret = IOS_Ioctl(di_fd, DVD_IDENTIFY, dic, 0x20, outbuf, 0x20);

    if (ret == 2)
        errno = EIO;

    memcpy(id, outbuf, sizeof(DI_DriveID));

    LWP_MutexUnlock(bufferMutex);

    return (ret == 1) ? 0 : -ret;
}

/*
 Returns the current error code on the drive.
 yagcd has a pretty comprehensive list of possible error codes
 */
int DI2_GetError(uint32_t* error)
{
    if (!error)
    {
        errno = EINVAL;
        return -1;
    }

    CheckAccess();
    LWP_MutexLock(bufferMutex);

    dic[0] = DVD_GET_ERROR << 24;

    int ret = IOS_Ioctl(di_fd, DVD_GET_ERROR, dic, 0x20, outbuf, 0x20);

    if (ret == 2)
        errno = EIO;

    *error = outbuf[0]; // Error code is returned as an int in the first four bytes of outbuf.

    LWP_MutexUnlock(bufferMutex);

    return (ret == 1) ? 0 : -ret;
}

/*
 Reset the drive.
 */
int DI2_Reset()
{
    LWP_MutexLock(bufferMutex);

    dic[0] = DVD_RESET << 24;
    dic[1] = 1;

    int ret = IOS_Ioctl(di_fd, DVD_RESET, dic, 0x20, outbuf, 0x20);

    if (ret == 2)
        errno = EIO;

    LWP_MutexUnlock(bufferMutex);

    return (ret == 1) ? 0 : -ret;
}

/*
 Main read function, basically just a wrapper to the function pointer.
 Nicer then just exposing the pointer itself
 */
int DI2_ReadDVD(void* buf, uint32_t len, uint32_t lba)
{
    int ret;

    if (DI2_ReadDVDptr)
    {
        CheckAccess();

        // Wait for the lock. Doing it here saves me from doing it in all the read functions.
        LWP_MutexLock(bufferMutex);
        ret = ReadBlockFromCache(buf, len, lba);
        LWP_MutexUnlock(bufferMutex);
        return ret;
    }

    return -1;
}

int DI2_ReadDVDAsync(void* buf, uint32_t len, uint32_t lba, ipccallback ipc_cb)
{
    int ret;

    if (DI2_ReadDVDAsyncptr)
    {
        CheckAccess();
        LWP_MutexLock(bufferMutex);
        ret = DI2_ReadDVDAsyncptr(buf, len, lba, ipc_cb);
        LWP_MutexUnlock(bufferMutex);
        return ret;
    }

    return -1;
}

/*
 Unknown what this does as of now...
 */
int DI2_ReadDVDConfig(uint32_t* val, uint32_t flag)
{
    if (!val)
    {
        errno = EINVAL;
        return -1;
    }

    CheckAccess();
    LWP_MutexLock(bufferMutex);

    dic[0] = DVD_READ_CONFIG << 24;
    dic[1] = flag & 0x1; // Update flag, val will be written if this is 1, val won't be written if it's 0.
    dic[2] = 0; // Command will fail driveside if this is not zero.
    dic[3] = *val;

    int ret = IOS_Ioctl(di_fd, DVD_READ_CONFIG, dic, 0x20, outbuf, 0x20);

    if (ret == 2)
        errno = EIO;

    *val = outbuf[0];

    LWP_MutexUnlock(bufferMutex);

    return (ret == 1) ? 0 : -ret;
}

/*
 Read the copyright information on a DVDVideo
 */
int DI2_ReadDVDCopyright(uint32_t* copyright)
{
    if (!copyright)
    {
        errno = EINVAL;
        return -1;
    }

    CheckAccess();
    LWP_MutexLock(bufferMutex);

    dic[0] = DVD_READ_COPYRIGHT << 24;
    dic[1] = 0;

    int ret = IOS_Ioctl(di_fd, DVD_READ_COPYRIGHT, dic, 0x20, outbuf, 0x20);
    *copyright = *((uint32_t*) outbuf); // Copyright information is returned as an int in the first four bytes of outbuf.

    if (ret == 2)
        errno = EIO;

    LWP_MutexUnlock(bufferMutex);

    return (ret == 1) ? 0 : -ret;
}

/*
 Returns 0x800 bytes worth of Disc key
 */
int DI2_ReadDVDDiscKey(void* buf)
{
    int ret;
    int retry_count = LIBDI_MAX_RETRIES;

    if (!buf)
    {
        errno = EINVAL;
        return -1;
    }

    if ((uint32_t) buf & 0x1F)
    {
        errno = EFAULT;
        return -1;
    }

    CheckAccess();
    LWP_MutexLock(bufferMutex);

    dic[0] = DVD_READ_DISCKEY << 24;
    dic[1] = 0; // Unknown what this flag does.

    do
    {
        ret = IOS_Ioctl(di_fd, DVD_READ_DISCKEY, dic, 0x20, buf, 0x800);
        retry_count--;
    }

    while (ret != 1 && retry_count > 0);

    if (ret == 2)
        errno = EIO;

    LWP_MutexUnlock(bufferMutex);

    return (ret == 1) ? 0 : -ret;
}

/*
 This function will read the initial sector on the DVD, which contains stuff like the booktype
 */
int DI2_ReadDVDPhysical(void* buf)
{
    int ret;
    int retry_count = LIBDI_MAX_RETRIES;

    if (!buf)
    {
        errno = EINVAL;
        return -1;
    }

    if ((uint32_t) buf & 0x1F)
    {
        errno = EFAULT;
        return -1;
    }

    CheckAccess();
    LWP_MutexLock(bufferMutex);

    dic[0] = DVD_READ_PHYSICAL << 24;
    dic[1] = 0; // Unknown what this flag does.

    do
    {
        ret = IOS_Ioctl(di_fd, DVD_READ_PHYSICAL, dic, 0x20, buf, 0x800);
        retry_count--;
    }

    while (ret != 1 && retry_count > 0);

    if (ret == 2)
        errno = EIO;

    LWP_MutexUnlock(bufferMutex);

    return (ret == 1) ? 0 : -ret;
}

int DI2_ReportKey(int keytype, uint32_t lba, void* buf)
{
    if (!buf)
    {
        errno = EINVAL;
        return -1;
    }

    if ((uint32_t) buf & 0x1F)
    {
        errno = EFAULT;
        return -1;
    }

    CheckAccess();
    LWP_MutexLock(bufferMutex);

    dic[0] = DVD_REPORTKEY << 24;
    dic[1] = keytype & 0xFF;
    dic[2] = lba;

    int ret = IOS_Ioctl(di_fd, DVD_REPORTKEY, dic, 0x20, buf, 0x20);

    if (ret == 2)
        errno = EIO;

    LWP_MutexUnlock(bufferMutex);

    return (ret == 1) ? 0 : -ret;
}

int DI2_GetCoverRegister(uint32_t* status)
{
    LWP_MutexLock(bufferMutex);

    memset(dic, 0x00, 0x20);

    int ret = IOS_Ioctl(di_fd, DVD_GETCOVER, dic, 0x20, outbuf, 0x20);

    if (ret == 2)
        errno = EIO;

    *status = outbuf[0];

    LWP_MutexUnlock(bufferMutex);

    return (ret == 1) ? 0 : -ret;
}

/* Internal function for controlling motor operations */
int _DI2_SetMotor(int flag)
{
    LWP_MutexLock(bufferMutex);
    LastAccess = ticks_to_secs(gettime());

    dic[0] = DVD_SET_MOTOR << 24;
    dic[1] = flag & 0x1; // Eject flag.
    dic[2] = (flag >> 1) & 0x1; // Don't use this flag, it kills the drive untill next reset.

    int ret = IOS_Ioctl(di_fd, DVD_SET_MOTOR, dic, 0x20, outbuf, 0x20);

    if (ret == 2)
        errno = EIO;

    if (ret == 1)
        motor_stopped = true;

    LWP_MutexUnlock(bufferMutex);

    return (ret == 1) ? 0 : -ret;
}

/* Stop the drives motor, needs a reset afterwards for normal operation */
int DI2_StopMotor()
{
    return _DI2_SetMotor(0);
}

/* Stop the motor, and eject the disc. Also needs a reset afterwards for normal operation */
int DI2_Eject()
{
    return _DI2_SetMotor(1);
}

/* Warning, this will kill your drive untill the next reset. Will not respond to DI commands,
 will not take in or eject the disc. Your drive will be d - e - d, dead.
 
 I deem this function to be harmless, as normal operation will resume after a reset.
 However, I am not liable for anyones drive exploding as a result from using this function.
 */
int DI2_KillDrive()
{
    return _DI2_SetMotor(2);
}

int DI2_ClosePartition()
{
    CheckAccess();
    LWP_MutexLock(bufferMutex);

    dic[0] = DVD_CLOSE_PARTITION << 24;

    int ret = IOS_Ioctl(di_fd, DVD_CLOSE_PARTITION, dic, 0x20, outbuf, 0x20);

    if (ret == 2)
        errno = EIO;

    LWP_MutexUnlock(bufferMutex);

    return (ret == 1) ? 0 : -ret;
}

int DI2_OpenPartition(uint32_t offset)
{
    static ioctlv vectors[5] __attribute__((aligned(32)));
    static char certs[0x49e4] __attribute__((aligned(32)));
    CheckAccess();
    LWP_MutexLock(bufferMutex);

    dic[0] = DVD_OPEN_PARTITION << 24;
    dic[1] = offset;

    vectors[0].data = dic;
    vectors[0].len = 0x20;
    vectors[1].data = NULL;
    vectors[1].len = 0x2a4;
    vectors[2].data = NULL;
    vectors[2].len = 0;

    vectors[3].data = certs;
    vectors[3].len = 0x49e4;
    vectors[4].data = outbuf;
    vectors[4].len = 0x20;

    int ret = IOS_Ioctlv(di_fd, DVD_OPEN_PARTITION, 3, 2, vectors);

    if (ret == 2)
        errno = EIO;

    LWP_MutexUnlock(bufferMutex);

    return (ret == 1) ? 0 : -ret;
}

int DI2_Read(void *buf, uint32_t size, uint32_t offset)
{
    if (!buf)
    {
        errno = EINVAL;
        return -1;
    }

    if ((uint32_t) buf & 0x1F)
    {
        errno = EFAULT;
        return -1;
    }

    CheckAccess();
    LWP_MutexLock(bufferMutex);

    dic[0] = DVD_LOW_READ << 24;
    dic[1] = size;
    dic[2] = offset;

    int ret = IOS_Ioctl(di_fd, DVD_LOW_READ, dic, 0x20, buf, size);

    if (ret == 2)
        errno = EIO;

    LWP_MutexUnlock(bufferMutex);

    return (ret == 1) ? 0 : -ret;
}

int DI2_UnencryptedRead(void *buf, uint32_t size, uint32_t offset)
{
    int ret, retry_count = LIBDI_MAX_RETRIES;

    if (!buf)
    {
        errno = EINVAL;
        return -1;
    }

    if ((uint32_t) buf & 0x1F)
    { // This only works with 32 byte aligned addresses!
        errno = EFAULT;
        return -1;
    }

    CheckAccess();
    LWP_MutexLock(bufferMutex);

    dic[0] = DVD_READ_UNENCRYPTED << 24;
    dic[1] = size;
    dic[2] = offset;

    do
    {
        ret = IOS_Ioctl(di_fd, DVD_READ_UNENCRYPTED, dic, 0x20, buf, size);
        retry_count--;
    }

    while (ret != 1 && retry_count > 0);

    if (ret == 2)
        errno = EIO;

    LWP_MutexUnlock(bufferMutex);

    return (ret == 1) ? 0 : -ret;
}

int DI2_ReadDiscID(uint64_t *id)
{
    CheckAccess();
    LWP_MutexLock(bufferMutex);

    dic[0] = DVD_READ_DISCID << 24;

    int ret = IOS_Ioctl(di_fd, DVD_READ_DISCID, dic, 0x20, outbuf, 0x20);

    if (ret == 2)
        errno = EIO;

    memcpy(id, outbuf, sizeof(*id));

    LWP_MutexUnlock(bufferMutex);

    return (ret == 1) ? 0 : -ret;
}

bool diio_Startup()
{
    return true;
}

bool diio_Shutdown()
{
    return true;
}

bool diio_ReadSectors(sec_t sector, sec_t numSectors, void *buffer)
{
    return true;
}

bool diio_WriteSectors(sec_t sector, sec_t numSectors, const void *buffer)
{
    return true;
}

bool diio_ClearStatus()
{
    return true;
}

bool diio_IsInserted()
{
    return true;
}

bool diio_IsInitialized()
{
    return true;
}

const DISC_INTERFACE __io_wiidvd2 =
    {
        DEVICE_TYPE_WII_DVD,
        FEATURE_MEDIUM_CANREAD | FEATURE_WII_DVD,
        (FN_MEDIUM_STARTUP) & diio_Startup,
        (FN_MEDIUM_ISINSERTED) & diio_IsInserted,
        (FN_MEDIUM_READSECTORS) & diio_ReadSectors,
        (FN_MEDIUM_WRITESECTORS) & diio_WriteSectors,
        (FN_MEDIUM_CLEARSTATUS) & diio_ClearStatus,
        (FN_MEDIUM_SHUTDOWN) & diio_Shutdown
    };
