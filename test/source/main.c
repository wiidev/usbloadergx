/************************************************************/
/*                   USB speed test                         */
/*                                                          */
/*                                                          */
/************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <sys/dir.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include <fcntl.h>
#include <debug.h>

#include <sdcard/gcsd.h>
#include <sdcard/wiisd_io.h>
#include <sdcard/gcsd.h>
#include <ogc/usbstorage.h>

#include <ogc/lwp_watchdog.h>

#include "mload.h"
#include "../build/ehcmodule_elf.h" 
//#include "../build/fat_elf.h"
data_elf my_data_elf;
int my_thread_id = 0;


#include <fat.h>

#include <sdcard/wiisd_io.h>
#include <sdcard/gcsd.h>
#include <ogc/usbstorage.h>
#include "usb2storage.h"



const DISC_INTERFACE* sd = &__io_wiisd;
const DISC_INTERFACE* usb = &__io_usbstorage;
//const DISC_INTERFACE* usb1 = &__io_usb1storage;

static bool reset_pressed = false;
static bool power_pressed = false;

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;
static const u32 CACHE_PAGES = 8;

static u8 memsector[4*1024];


void initialise_video()
{
    VIDEO_Init();
    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

    printf("\x1b[2;0H");

    VIDEO_WaitVSync();
}

#include<ogc/lwp_watchdog.h> 
//#define ticks_to_secs(ticks)  ((u32)((u64)(ticks)/(u64)(TB_TIMER_CLOCK*1000)))
//#define ticks_to_msecs(ticks)  ((u32)((u64)(ticks)/(u64)(TB_TIMER_CLOCK)))

void showdir(char *path)
{
    char filename[MAXPATHLEN];
    DIR_ITER *dp;

    struct stat fstat;
    //chdir("usb:/");
    dp = diropen(path);

    if (dp == NULL)
    {
        printf("Error diropen\n");
        return ;
    }

    else printf("Ok diropen\n");

    int cnt = 0;

    while ( dirnext( dp, filename, &fstat ) == 0 )
    {
        //if(cnt==20) break;
        cnt++;

        if ( fstat.st_mode & S_IFDIR ) printf("/");

        printf("%s\n", filename);

    }

    dirclose(dp);
    printf("\nEnd show dirrectory (%d).\n", cnt);
}

static void reset_cb (void)
{
    reset_pressed = true;
}

static void power_cb (void)
{
    power_pressed = true;
}

#include <errno.h>


#define VERSION "1.52"

void save_log() // at now all data is sent to usbgecko, sd log disabled
{
    FILE *fp;
    u8 *temp_data = NULL;
    u32 level, len;

    temp_data = memalign(32, 256 * 1024);

    mload_seek(0x13750000, SEEK_SET);
    mload_read(temp_data, 128*1024);

    //mload_init();

    //mload_close();

    for (len = 0;len < 4096 && temp_data[len] != 0;len++);

    fatUnmount("sd");

    if (fatMount("sd", sd, 0, 128, 2) )
    {

        fp = fopen("sd:/log_usb2.txt", "wb");

        if (fp != 0)
        {
            fprintf(fp, "USB2 device test program. v%s\n------------------------------\n", VERSION);
            fwrite(temp_data, 1, len , fp);

            fclose(fp);
        }
    }

    if (usb_isgeckoalive(1))
    {
        level = IRQ_Disable();
        usb_sendbuffer(1, temp_data, len);
        IRQ_Restore(level);
    }

    //printf("\nlog:\n-------\n%s", temp_data);
    free(temp_data);
}

void TryUSBDevice()
{
    printf("Initializing usb device.\n");
    bool s;

    USB2Enable(true);
    s = usb->startup();
    usleep(10000);

    if (s > 0)
        s = __usb2storage_ReadSectors(0, 1, memsector);

    usleep(10000);


    if (s > 0)
    {

        printf("usb device detected, your device is fully compatible with this usb2 driver\n");
    }

    else
    {
        //(s==-1) printf("usb device NOT INSERTED, log file not created\n");
        //se
        {
            printf("usb device NOT detected or not inserted, report 'ubs2 init' value  and log file\n");
            printf("Log saved to sd:/log_usb2.txt file\n");
        }
    }

    save_log();
}

static s32 initialise_network()
{
    s32 result;

    while ((result = net_init()) == -EAGAIN)
    {
        if (reset_pressed) exit(1);

        usleep(100);
    }

    return result;
}

int wait_for_network_initialisation()
{
    printf("Waiting for network to initialise...\n");

    if (initialise_network() >= 0)
    {
        char myIP[16];

        if (if_config(myIP, NULL, NULL, true) < 0)
        {
            printf("Error reading IP address\n");
            return 0;
        }

        else
        {
            printf("Network initialised.  Wii IP address: %s\n", myIP);
            return 1;
        }
    }

    printf("Unable to initialise network\n");
    return 0;
}

#define ROUNDDOWN32(v)    (((u32)(v)-0x1f)&~0x1f)
static bool usblan_detected(void)
{
    u8 *buffer;
    u8 dummy;
    u8 i;
    u16 vid, pid;

    USB_Initialize();

    buffer = (u8*)ROUNDDOWN32(((u32)SYS_GetArena2Hi() - (32 * 1024)));

    memset(buffer, 0, 8 << 3);

    if (USB_GetDeviceList("/dev/usb/oh0", buffer, 8, 0, &dummy) < 0)
    {
        return false;
    }

    for (i = 0; i < 8; i++)
    {
        memcpy(&vid, (buffer + (i << 3) + 4), 2);
        memcpy(&pid, (buffer + (i << 3) + 6), 2);

        if ( (vid == 0x0b95) && (pid == 0x7720))
        {
            return true;
        }
    }

    return false;
}

void init_mload()
{
    int ret;
    ret = mload_init();

    if (ret < 0)
    {
        printf("fail to get dev/mload, mload cios not detected. Quit\n");
        sleep(3);
        exit(0);
        //goto out;
    }
}

void loadehcimodule()
{
    u32 addr;
    int len;

    mload_get_load_base(&addr, &len);

    if (((u32) ehcmodule_elf) & 3) {printf("Unaligned elf!\n"); return ;}

    mload_elf((void *) ehcmodule_elf, &my_data_elf);
    my_thread_id = mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack,  /*my_data_elf.prio*/0x47);
    //my_thread_id=mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, my_data_elf.prio);

    if (my_thread_id < 0)
    {
        printf("EHCI module can NOT be loaded (%i).. Quit\n", my_thread_id);
        sleep(3);
        exit(0);
    }

    printf("EHCI module loaded (%i).\n", my_thread_id);
    VIDEO_WaitVSync();
}

int main(int argc, char **argv)
{
    int ret;
    bool usblan;
    int usblanport;

    initialise_video();
    CON_EnableGecko(1, 0);

    usblan = usblan_detected();
    printf("ios 202 reload\n");
    IOS_ReloadIOS(202);
    printf("ios 202 reloaded\n");
    init_mload();
    mload_reset_log();
    loadehcimodule();

    if (usblan)
    {
        printf("uslan detected\n");
        usblanport = GetUSB2LanPort();
        printf("usblanport: %i\n", usblanport);
        USB2Storage_Close();

        mload_close();
        IOS_ReloadIOS(202);
        init_mload();
        loadehcimodule();
        printf("SetUSBMode(%i)\n", usblanport);

        if (usblanport == 1)
            SetUSB2Mode(1);
        else
            SetUSB2Mode(2);

    }

    else
        printf("usblan NOT found\n");

    SYS_SetResetCallback (reset_cb);

    SYS_SetPowerCallback (power_cb);

    //  usleep(10000);
    //WIIDVD_Init(false);
    //  usleep(10000);

    TryUSBDevice(); // init usb2 device

    // usleep(10000);
    // save_log();
    // usleep(10000);
    /*
     if(WIIDVD_DiscPresent()) printf("dvd inserted\n");
     else printf("dvd NOT inserted\n");
     usleep(10000);
     save_log();
     
     if(fatMount("usb",usb,0,3,128))
     {
      showdir("usb:/");
     }
     else printf("can't mount usb\n"); 
     
     sleep(6);
     return 0;
     wait_for_network_initialisation(); // check if lan is working
     sleep(1);
    */
    printf("Press Home or reset to exit. Press A or power to try to detect usb device again.\n");

    VIDEO_WaitVSync();

    VIDEO_WaitVSync();

    while (1) //wait home press to exit
    {
        WPAD_ScanPads();

        if (reset_pressed) break;

        u32 pressed = WPAD_ButtonsDown(0);

        if ( pressed & WPAD_BUTTON_HOME )
        {
            mload_close();
            printf ("\n\nExit ...");
            exit(0);
        }

        if ( (pressed & WPAD_BUTTON_A) || power_pressed)
        {
            power_pressed = false;

            TryUSBDevice();
            //usleep(5000);
            power_pressed = false;
            /*
            if(fatMount("usb",usb,0,3,128))
            {
             showdir("usb:/");
            }
            else printf("can't mount usb\n"); 
            */ 
            //TryUSBDevice();
            /*
            printf("deinit (5secs)\n");
            sleep(1);
            net_deinit();
            sleep(4);
            printf("init\n");
            wait_for_network_initialisation(); 
            */ 
            //power_pressed=false;
        }

        usleep(5000);
        VIDEO_WaitVSync();
    }

out:
    mload_close();
    printf ("\n\nExit ...");
    exit(0);
}
