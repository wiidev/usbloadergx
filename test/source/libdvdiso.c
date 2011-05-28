#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#ifndef DEBUG
#include <sys/iosupport.h>
#include <ogc/ipc.h>
#include <gccore.h>
#include <ogc/mutex.h>
#include <ogc/lwp_watchdog.h>
#endif
#include "libdvdiso.h"
#include "di2.h"

#define MAXPATH 4096
#define MAXNAMELEN 208


#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int
static mutex_t _DVD_mutex = LWP_MUTEX_NULL;
static bool dvd_initied = false;

static int totalsectors;
static int totalentries;

static char currentpath[MAXPATH];

static void freedirentrieslist(void);
static void dotab_dvd_add(void);
static int DVD_ScanContent(void);

struct DIRENTRY
{
    char name[MAXNAMELEN + 1];
    uint32_t size_bytes;
    uint32_t size_sectors;
    uint32_t firstsector;
    uint8_t isfile;
    uint8_t isdir;
    uint8_t ishidden;

    struct DIRENTRY *next;

    struct DIRENTRY *parent;

    struct DIRENTRY *child;
    uint32_t fake_inode;
};

static struct DIRENTRY *direntriesptr;

struct FILESTATE
{

    struct DIRENTRY *dentry;
    uint32_t firstsector;
    uint32_t pos_bytes;
    uint32_t size_bytes;
    uint32_t localsectnum;
    uint8_t *localsectbuf;
    int fd;
};

struct DIR_STATE
{

    struct DIRENTRY *firstdentry;

    struct DIRENTRY *curdentry;
    uint8_t inUse;
};

#ifdef DEBUG
FILE *fpin;

struct _reent
{
    int _errno;
};

uint32_t fake_ticks = 1000;
uint32_t gettick(){ return fake_ticks++;};

#endif


///////////////////////////////////////////
//      CACHE FUNCTION DEFINITIONS       //
///////////////////////////////////////////
#define CACHE_FREE 0xFFFFFFFF

typedef struct
{
    uint32_t sector;
    uint32_t last_used;
    void *ptr;
}

cache_page;

static cache_page *ReadAheadCache = NULL;
static uint32_t RA_pages = 0;
static uint32_t RA_sectors = 0;
#define SECTOR_SIZE 0x800

static void DestroyReadAheadCache();
static int ReadSectorFromCache(void *buf, uint32_t sector);
static void DVDEnableReadAhead(uint32_t pages, uint32_t sectors);

///////////////////////////////////////////
//    END CACHE FUNCTION DEFINITIONS     //
///////////////////////////////////////////

/*
void dump_hex(uint8_t *ptr, int len)
{
 int i,pos=0;
 do
 {
  for(i=0;(i<8)&&(pos<len);i++)
  {
   printf(" %02X",ptr[pos++]);
  }
  printf("\n");
 }
 while(pos<len);
}
*/

int WIIDVD_Init(bool dvdx)
{
    int retval;
    direntriesptr = 0;
    totalsectors = 0;
    currentpath[0] = '/';
    currentpath[1] = 0;
    totalentries = 0;

#ifndef DEBUG
    retval = DI2_Init(dvdx);
    printf("retval: DI2_Init: %i\n", retval);

    if (retval >= 0) dvd_initied = true;
    else dvd_initied = false;

#else
    fpin = fopen("/dev/sr0", "rb");

    if (!fpin)
    {
        printf("Error while opening device or file\n");
        return -1;
    }

#endif

#ifndef DEBUG
    dotab_dvd_add();

#endif
    LWP_MutexInit(&_DVD_mutex, false);

    return retval;
}

void WIIDVD_Close()
{
    freedirentrieslist();
    DestroyReadAheadCache();
#ifndef DEBUG
    DI2_Close();
#endif
    LWP_MutexDestroy(_DVD_mutex);
}

int WIIDVD_Mount()
{
#ifndef DEBUG
    DI2_Mount();
    unsigned int t1, t2;
    t1 = ticks_to_secs(gettime());

    while (DI2_GetStatus() & DVD_INIT)
    {
        t2 = ticks_to_secs(gettime());

        if (t2 - t1 > 15) return -1;

        usleep(5000);
    }

#endif
    DVDEnableReadAhead(10, 26);  //default init

    return DVD_ScanContent();
}

void WIIDVD_Unmount()
{
    freedirentrieslist();
    DestroyReadAheadCache();
}


int WIIDVD_ReadDVD(void* buf, uint32_t len, uint32_t lba)
{
    int retval;
#ifdef DEBUG 
    // printf("WIIDVD_ReadDVD: Reading from sector %u len %u\n",lba,len);

    retval = fseek(fpin, lba * 2048, SEEK_SET);

    if (retval)printf("  fseek returned %d\n", retval);

    retval = fread(buf, len, 2048, fpin);

    if (retval != 2048)printf("  fread returned %d\n", retval);

    return 0;

#else
    retval = DI2_ReadDVD(buf, len, lba);

    //if(retval)printf("Error %d reading sectors %d->%d\n",retval,lba,lba+len-1);
    return retval;

#endif
}

int WIIDVD_DiscPresent()
{
#ifdef DEBUG
    return 1;
#else
    uint32_t val;

    if (!dvd_initied) return 0;

    DI2_GetCoverRegister(&val);

    if (val&0x2) return 1;

    return 0;

#endif
}

static struct DIRENTRY * name_to_dentry(char *filename, uint8_t file_permitted, uint8_t dir_permitted) //filename must be absolute, without dvd: at beginning
{
    //int done=0;

    struct DIRENTRY * dentryptr;
    char *charptr, *nextcharptr;
    char nametofind[MAXPATH];
    uint8_t found;

    if (filename[0] == '/')filename++; //Skip initial /

    strcpy(nametofind, filename);

    //printf("name_to_dentry: '%s'\n",nametofind);

    if (!direntriesptr) return NULL;

    dentryptr = direntriesptr->child;

    if (!dentryptr) return NULL;

    charptr = strtok(nametofind, "/");

    //printf("charptr: '%s'\n",charptr);
    if (charptr == NULL)
    {
        return direntriesptr;
    }

    do
    {
        if (charptr == NULL) return NULL;

        nextcharptr = strtok(NULL, "/");

        //  printf("searching for: '%s'\n",charptr);

        found = 0;

        do
        {
            //   printf("Comparing '%s' with '%s'\n",dentryptr->name,charptr);

            if (strcmp(dentryptr->name, charptr) == 0)
            {
                found = 1;

                if (nextcharptr) //non-last path item
                {

                    if (!dentryptr->isdir) return NULL; //we are searching for a file inside this non-directory item

                    //     printf("Is a directory because the following item is '%s'\n",nextcharptr);
                    dentryptr = dentryptr->child;

                    if (!dentryptr) return NULL;

                    //     printf("Found child '%s'\n",dentryptr->name);
                }
                else //last path item
                {
                    if ((dentryptr->isfile) && (!file_permitted)) return NULL; //we don't want files

                    if ((dentryptr->isdir) && (!dir_permitted)) return NULL; //we don't want directory

                    //     if(dentryptr->isfile)printf("Found file '%s'\n",dentryptr->name);
                    //     if(dentryptr->isdir)printf("Found directory '%s'\n",dentryptr->name);
                    return dentryptr;
                }

            }

            else
            {
                dentryptr = dentryptr->next;
            }
        }

        while ((!found) && (dentryptr));

        if (!found) return NULL;

        charptr = nextcharptr;
    }

    while (dentryptr);

    return NULL;
}

static char *absolute_path_without_device(const char *srcpath, char *destpath)
{
    //Thanks to Chishm (libfat)
    // Move the path pointer to the start of the actual path

    if (strchr (srcpath, ':') != NULL)
    {
        srcpath = strchr (srcpath, ':') + 1;
    }

    if (strchr (srcpath, ':') != NULL)
    {
        return NULL;
    }

    if (srcpath[0] != '/') //relative path
    {
        strcpy(destpath, currentpath);
        strcat(destpath, srcpath);
    }
    else
    {
        strcpy(destpath, srcpath);
    }

    return destpath;
}

#ifndef DEBUG

static int dentrystat(struct DIRENTRY *dentry, struct stat *st)
{
    if (!st) return -1;

    if (!dentry) return -1;

    st->st_dev = 0; //?!??!?

    st->st_ino = (ino_t)dentry->fake_inode;

    // st->st_mode=(dentry->isdir ? S_IFDIR : S_IFREG) | (S_IRUSR | S_IRGRP | S_IROTH);
    st->st_mode = (dentry->isdir ? S_IFDIR : S_IFREG);

    st->st_nlink = 1;        // Always one hard link on a ISO entry

    st->st_uid = 1;         // Faked

    st->st_rdev = st->st_dev;

    st->st_gid = 2;         // Faked

    st->st_size = dentry->size_bytes;

    st->st_atime = 0; //FIXME!

    // st->st_spare1 = 0;
    st->st_mtime = 0; //FIXME;

    // st->st_spare2 = 0;
    st->st_ctime = 0; //FIXME

    // st->st_spare3 = 0;
    st->st_blksize = 0x800;

    st->st_blocks = dentry->size_sectors;

    st->st_spare4[0] = 0;

    st->st_spare4[1] = 0;

    return 0;
}

#endif

#ifndef DEBUG

static int _DVD_dirclose_r (struct _reent *r, DIR_ITER *dirState)
{

    struct DIR_STATE * mydirstate = (struct DIR_STATE *)dirState->dirStruct;

    // printf("DVD_dirclose\n");

    if (!(mydirstate->inUse))
    {
        r->_errno = EBADF;
        return -1;
    }

    memset(mydirstate, 0, sizeof(struct DIR_STATE));
    mydirstate->inUse = 0;
    return 0;
}

static int _DVD_dirnext_r (struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat)
{

    struct DIR_STATE * mydirstate = (struct DIR_STATE *)dirState->dirStruct;

    // printf("DVD_dirnext\n");

    if (!(mydirstate->inUse))
    {
        r->_errno = EBADF;
        return -1;
    }

    if (mydirstate->curdentry == NULL)
    {
        r->_errno = ENOENT;
        return -1;
    }

    // skip .. at root
    if (mydirstate->curdentry->fake_inode == 4 && strcmp(mydirstate->curdentry->name, "..") == 0)
    {
        mydirstate->curdentry = mydirstate->curdentry->next;

        if (mydirstate->curdentry == NULL)
        {
            r->_errno = ENOENT;
            return -1;
        }
    }

    strncpy(filename, mydirstate->curdentry->name, MAXNAMELEN);

    if (filestat != NULL)
    {
        dentrystat(mydirstate->curdentry, filestat);
    }

    mydirstate->curdentry = mydirstate->curdentry->next;

    return 0;
}

static int _DVD_dirreset_r (struct _reent *r, DIR_ITER *dirState)
{

    struct DIR_STATE * mydirstate = (struct DIR_STATE *)dirState->dirStruct;

    // printf("DVD_dirreset\n");

    if (!(mydirstate->inUse))
    {
        r->_errno = EBADF;
        return -1;
    }

    mydirstate->curdentry = mydirstate->firstdentry;
    return 0;
}

static DIR_ITER* _DVD_diropen_r(struct _reent *r, DIR_ITER *dirState, const char *path)
{
    char path_absolute[MAXPATH];

    struct DIR_STATE * mydirstate = (struct DIR_STATE *)dirState->dirStruct;

    struct DIRENTRY * dentry;

    // printf("DVD_diropen\n");

    if (absolute_path_without_device(path, path_absolute) == NULL)
    {
        r->_errno = EINVAL;
        return NULL;
    }

    dentry = name_to_dentry(path_absolute, 1, 1); //filename must be absolute, without dvd: at beginning

    if (!dentry)
    {
        r->_errno = ENOENT;
        return NULL;
    }

    if (!(dentry->isdir))
    {
        r->_errno = ENOTDIR;
        return NULL;
    }

    mydirstate->firstdentry = dentry->child;
    mydirstate->curdentry = dentry->child;
    mydirstate->inUse = 1;

    // printf("firstdentry: '%s'\n",dentry->name);
    return dirState;
}

static int _DVD_statvfs_r (struct _reent *r, const char *path, struct statvfs *buf)
{
    // printf("DVD_statvfs\n");

    // FAT clusters = POSIX blocks
    buf->f_bsize = 0x800;  // File system block size.
    buf->f_frsize = 0x800; // Fundamental file system block size.

    buf->f_blocks = totalsectors; // Total number of blocks on file system in units of f_frsize.
    buf->f_bfree = 0; // Total number of free blocks.
    buf->f_bavail = 0; // Number of free blocks available to non-privileged process.

    // Treat requests for info on inodes as clusters
    buf->f_files = totalentries; // Total number of file serial numbers.
    buf->f_ffree = 0; // Total number of free file serial numbers.
    buf->f_favail = 0; // Number of file serial numbers available to non-privileged process.

    // File system ID. 32bit ioType value
    buf->f_fsid = 0; //??!!?

    // Bit mask of f_flag values.
    buf->f_flag = ST_NOSUID // No support for ST_ISUID and ST_ISGID file mode bits
                  | ST_RDONLY ; // Read only file system
    // Maximum filename length.
    buf->f_namemax = MAXNAMELEN;
    return 0;
}

#endif

static int _DVD_chdir_r (struct _reent *r, const char *path)
{
    char path_absolute[MAXPATH];

    struct DIRENTRY * dentry;

    // printf("DVD_chdir\n");

    if (absolute_path_without_device(path, path_absolute) == NULL)
    {
        r->_errno = EINVAL;
        return -1;
    }

    // printf("path_absolute: %s\n",path_absolute);
    dentry = name_to_dentry(path_absolute, 1, 1); //filename must be absolute, without dvd: at beginning

    if (!dentry)
    {
        r->_errno = ENOENT;
        return -1;
    }

    if (!(dentry->isdir))
    {
        r->_errno = ENOTDIR;
        return -1;
    }

    strcpy(currentpath, path_absolute);

    if (currentpath[0] != 0)
    {
        if (currentpath[strlen(currentpath) - 1] != '/')strcat(currentpath, "/");
    }

    return 0;
}

#ifndef DEBUG

static int _DVD_stat_r (struct _reent *r, const char *path, struct stat *st)
{
    char path_absolute[MAXPATH];

    struct DIRENTRY * dentry;

    // printf("DVD_stat\n");

    if (absolute_path_without_device(path, path_absolute) == NULL)
    {
        r->_errno = EINVAL;
        return -1;
    }

    dentry = name_to_dentry(path_absolute, 1, 1); //filename must be absolute, without dvd: at beginning

    if (!dentry)
    {
        r->_errno = ENOENT;
        return -1;
    }

    dentrystat(dentry, st);
    return 0;
}

static int _DVD_fstat_r (struct _reent *r, int fd, struct stat *st)
{

    struct FILESTATE *filestate = (struct FILESTATE *)fd;

    // printf("DVD_fstat\n");


    if (!filestate)
    {
        r->_errno = EBADF;
        return -1;
    }

    if (!(filestate->dentry->isfile))
    {
        r->_errno = EISDIR;
        return -1;
    }

    st->st_ino = filestate->dentry->fake_inode;
    st->st_size = filestate->dentry->size_bytes;
    return 0;
}

#endif

static off_t _DVD_seek_r (struct _reent *r, int fd, off_t pos, int dir)
{

    struct FILESTATE *filestate = (struct FILESTATE *)fd;
    int newpos = 0;

    // printf("DVD_seek fd: %d pos:%d ",fd,pos);
    // if(dir==SEEK_SET)printf("SEEK_SET\n");
    // if(dir==SEEK_CUR)printf("SEEK_CUR\n");
    // if(dir==SEEK_END)printf("SEEK_END\n");

    if (!filestate)
    {
        r->_errno = EBADF;
        return -1;
    }

    if (dir == SEEK_SET)
    {
        newpos = pos;
    }

    else if (dir == SEEK_CUR)
    {
        newpos = pos + (filestate->pos_bytes);

        if (newpos > (filestate->size_bytes))newpos = filestate->size_bytes;

        if (newpos < (filestate->pos_bytes))
        {
            r->_errno = EOVERFLOW;
            return -1;
        }
    }

    else if (dir == SEEK_END)
    {
        newpos = (filestate->size_bytes) - pos;

        if (newpos < 0)
        {
            r->_errno = EINVAL;
            return -1;
        }
    }

    else
    {
        r->_errno = EINVAL;
        return -1;
    }

    // printf(" pos:%d newpos:%d\n",pos,newpos);
    filestate->pos_bytes = newpos;

    return newpos;
}

static ssize_t _DVD_read_r (struct _reent *r, int fd, char *ptr, size_t len)
{

    struct FILESTATE *filestate = (struct FILESTATE *)fd;
    int bytestoread, bytesread, smallread, localpos, sectorneeded, retval;


    if (!filestate)
    {
        r->_errno = EBADF;
        return -1;
    }

    // printf("DVD_read fd: %d curpos: %d :len:%d ",fd,filestate->pos_bytes,len);

    if ((len + (filestate->pos_bytes)) > (filestate->size_bytes))
    {
        len = (filestate->size_bytes) - (filestate->pos_bytes);

        if (len < 0)len = 0;

        r->_errno = EOVERFLOW;

        if (len == 0) return 0;
    }

    bytestoread = len;
    bytesread = 0;

    do
    {
        smallread = bytestoread;

        sectorneeded = ((filestate->pos_bytes) >> 11) + (filestate->firstsector);
        localpos = (filestate->pos_bytes) & 0x7FF;

        if (smallread > (0x800 - localpos))smallread = 0x800 - localpos;


        if (filestate->localsectbuf == NULL)
        {
            filestate->localsectbuf = (uint8_t *)memalign(32, 0x800);

            if (filestate->localsectbuf == NULL)
            {
                r->_errno = ENOMEM;
                return -1;
            }

            filestate->localsectnum = sectorneeded + 1; //make sure we don't use this empty buffer!
        }

        if (sectorneeded != filestate->localsectnum)
        {
            filestate->localsectnum = sectorneeded;
            retval = ReadSectorFromCache(filestate->localsectbuf, sectorneeded);

            if (retval)
            {
                r->_errno = EIO;
                return -1;
            }
        }

        //  printf("bytestoread: %d localpos: %d, smallread: %d\n",bytestoread,localpos,smallread);
        //  dump_hex(&(filestate->localsectbuf[localpos]),smallread);
        memcpy(ptr, (filestate->localsectbuf) + localpos, smallread);

        ptr += smallread;

        bytestoread -= smallread;

        bytesread += smallread;

        filestate->pos_bytes += smallread;
    }

    while (bytestoread > 0);

    // printf("Read %d bytes\n",bytesread);
    return bytesread;
}

static int _DVD_close_r (struct _reent *r, int fd)
{

    struct FILESTATE *filestate = (struct FILESTATE *)fd;
    // printf("DVD_close\n");

    if (filestate == NULL)
    {
        r->_errno = ENOENT;
        return -1;
    }

    if (filestate->localsectbuf)free(filestate->localsectbuf);

    return 0;
}

int _DVD_open_r (struct _reent *r, void *fileStruct, const char *path, int flags, int mode)
{

    struct FILESTATE *filestate = (struct FILESTATE *)fileStruct;
    char file_absolute[MAXPATH];

    struct DIRENTRY * dentry;

    // printf("DVD_open\n");

    if (!WIIDVD_DiscPresent())
    {
        r->_errno = ENODEV;
        return -1;
    }

    if (absolute_path_without_device(path, file_absolute) == NULL)
    {
        r->_errno = EINVAL;
        return -1;
    }

    // Determine which mode the file is openned for
    if ((flags & 0x03) != O_RDONLY)
    {
        r->_errno = EROFS;
        return -1;
    }

    // printf("Trying to open '%s'\n",file_absolute);
    dentry = name_to_dentry(file_absolute, 1, 1); //filename must be absolute, without dvd: at beginning

    if (!dentry)
    {
        r->_errno = ENOENT;
        return -1;
    }

    if (dentry->isdir)
    {
        r->_errno = EISDIR;
        return -1;
    }

    filestate->dentry = dentry;
    filestate->pos_bytes = 0;
    filestate->size_bytes = dentry->size_bytes;
    filestate->firstsector = dentry->firstsector;
    filestate->localsectnum = 0;
    filestate->localsectbuf = NULL;

    // printf("Opened file '%s', firstsector: %d\n",filestate->dentry->name,filestate->firstsector);
    return (int)filestate;
}

static uint32_t bothendian32_to_uint32(uint8_t *buff)
{
    uint32_t val;
    val = ((uint32_t)buff[4]) << 24;
    val |= ((uint32_t)buff[5]) << 16;
    val |= ((uint32_t)buff[6]) << 8;
    val |= ((uint32_t)buff[7]);
    return val;
}

/*
static uint32_t bigendian32_to_uint32(uint8_t *buff)
{
 uint32_t val;
 val=((uint32_t)buff[0])<<24;
 val|=((uint32_t)buff[1])<<16;
 val|=((uint32_t)buff[2])<<8;
 val|=((uint32_t)buff[3]);
 return val;
}
 
static uint16_t bigendian16_to_uint16(uint8_t *buff)
{
 uint16_t val;
 val=((uint16_t)buff[0])<<8;
 val|=((uint16_t)buff[1]);
 return val;
}
*/

static void freedirentryandchilds(struct DIRENTRY *dentry)
{

    struct DIRENTRY *dentrynext;

    if (!dentry) return ;

    do
    {
        dentrynext = dentry->next;

        if (dentry->child)freedirentryandchilds(dentry->child);

        free(dentry);

        dentry = dentrynext;
    }

    while (dentry);

}

static void freedirentrieslist()
{
    freedirentryandchilds(direntriesptr);
    direntriesptr = 0;
}

#ifdef DEBUG

static void debug_dump_direntry(int tablevel, struct DIRENTRY *dentry)
{
    char spaces[33] = "                                ";
    spaces[tablevel*2] = 0;

    if (!dentry)
    {
        printf("%sNull Pointer!!!\n", spaces);
    }


    printf("%sName: '%s' FirstSector: %d isfile: %d isdir: %d ishidden: %d\r\n", spaces, dentry->name, dentry->firstsector, dentry->isfile, dentry->isdir, dentry->ishidden);

}

static void debug_dump_tree_recurse(int level, struct DIRENTRY *dentry, char *path_parent)
{
    char path[MAXPATH];
    char spaces[33] = "                                ";
    spaces[level*2] = 0;

    printf("%sPath: %s\n", spaces, path_parent);

    do
    {
        if (dentry->isdir)
        {
            if (dentry->child)
            {
                strcpy(path, path_parent);
                strcat(path, dentry->name);
                strcat(path, "/");
                debug_dump_tree_recurse(level + 1, dentry->child, path);
            }
        }

        else
        {
            printf("%sFile: %s%s\n", spaces, path_parent, dentry->name);
            //   debug_dump_direntry(level,dentry);
        }

        dentry = dentry->next;
    }

    while (dentry);

}

static void debug_dump_tree()
{
    if (direntriesptr)debug_dump_tree_recurse(0, direntriesptr, "dvd:/");
}

#endif

static int DVD_ScanContent_recurse(int jolietmode, struct DIRENTRY * parent, uint8_t level, uint32_t sectnum, uint32_t dirlen, int *fake_inodecounter)
{
    int retval, i;
    uint8_t * ptr, avoid_recursion;
    uint32_t sect_from, sect_to, sect, pos;
    uint8_t len;
    uint8_t rockridge_enabled = 0;
    uint8_t rockridge_bytestoskip = 0;

    struct DIRENTRY *direntryptr, *direntryptr_prev;
    uint8_t *localsectbuf;

    localsectbuf = (uint8_t *)memalign(32, 0x800);

    if (!localsectbuf) return -1;

    sect_from = sectnum;

    sect_to = sectnum + ((dirlen - 1) >> 11);

    // for(i=0;i<level;i++)printf("  ");printf("Start sector: %d End Sector: %d\n",sect_from,sect_to);

    direntryptr = 0;

    direntryptr_prev = 0;

    for (sect = sect_from;sect <= sect_to;sect++)
    {
        //  for(i=0;i<=level;i++)printf("  ");printf("Read sector %u\n",sect);
        retval = ReadSectorFromCache(localsectbuf, sect);

        if (retval != 0)
        {
            //for(i=0;i<=level;i++)printf("  ");printf("Error reading sector %d\n",sect);
            return -1;
        }

        pos = 0;

        do
        {
            uint8_t flags, namelen;

            ptr = &localsectbuf[pos];
            len = ptr[0];
            namelen = ptr[32];
            //   printf("len: %d namelen:%d\n",len,namelen);

            if (len && namelen)
            {

                direntryptr = (struct DIRENTRY *)malloc(sizeof(struct DIRENTRY));

                if (direntryptr == NULL) return -1;

                totalentries++;

                memset(direntryptr, 0, sizeof(struct DIRENTRY));

                *fake_inodecounter = (*fake_inodecounter) + 1;

                direntryptr->fake_inode = *fake_inodecounter;

                direntryptr->firstsector = bothendian32_to_uint32(&ptr[2]);

                direntryptr->size_bytes = bothendian32_to_uint32(&ptr[10]);

                direntryptr->size_sectors = ((direntryptr->size_bytes) - 1) >> 11;

                flags = ptr[25];

                direntryptr->next = NULL;

                direntryptr->isfile = (flags & 0x02) ? 0 : 1;

                direntryptr->isdir = (flags & 0x02) ? 1 : 0;

                direntryptr->ishidden = (flags & 0x01) ? 1 : 0;

                avoid_recursion = 0;

                namelen = ptr[32];

                if (jolietmode)
                {
                    //     if(namelen>128)return -1;

                    //     printf("namelen: %d\n",namelen);
                    //     dump_hex(&ptr[33],32);

                    if (namelen == 1)
                    {
                        if ((ptr[33] == 0)) // . (self)
                        {
                            strcpy(direntryptr->name, ".");
                            avoid_recursion = 1;
                        }
                        if ((ptr[33] == 1)) // .. (parent)
                        {
                            strcpy(direntryptr->name, "..");
                            avoid_recursion = 1;
                        }
                    }

                    else //if namelen != 1
                    {
                        uint8_t *charptr;
                        namelen >>= 1;

                        charptr = &ptr[33];

                        for (i = 0;i < namelen;i++)
                        {
                            if ((charptr[0] == 0) && (charptr[1] >= 32) && (charptr[1] < 0x80))
                                direntryptr->name[i] = charptr[1];
                            else
                                direntryptr->name[i] = '_'; //we don't support multi-byte characters or extended ascii characters, so replace the strange things with this symbol

                            charptr += 2;
                        }

                        direntryptr->name[i] = 0;
                    }
                }

                else //non-joliet mode, so ISO9660:1999 mode with or without RockRidge extension
                {
                    unsigned int bytesused;

                    if (namelen > 207) return -1;

                    memcpy(direntryptr->name, &ptr[33], namelen);

                    bytesused = 33 + namelen;

                    if ((namelen&0x01) == 0)bytesused++; //padding

                    if ((namelen == 1) && (direntryptr->name[0] == 0) && (direntryptr->name[1] == 0)) // . (self)
                    {
                        strcpy(direntryptr->name, ".");
                        avoid_recursion = 1;

                        if ((level == 0) && (!jolietmode))
                        {
                            if ((ptr[bytesused] == 'S') && (ptr[bytesused + 1] == 'P') && (ptr[bytesused + 2] == 7) && (ptr[bytesused + 3] == 1) && (ptr[bytesused + 4] == 0xBE) && (ptr[bytesused + 5] == 0xEF))
                            {
                                rockridge_enabled = 1;
                                rockridge_bytestoskip = ptr[bytesused + 6];
                            }
                        }

                    }

                    if ((namelen == 1) && (direntryptr->name[0] == 1) && (direntryptr->name[1] == 0)) // .. (parent)
                    {
                        strcpy(direntryptr->name, "..");
                        avoid_recursion = 1;
                    }


                    //check for RockRidge extension
                    if ((len > bytesused) && (rockridge_enabled)) //we have other data in System Use field
                    {

                        uint8_t *RRAttribute_ptr;
                        uint8_t RRAttribute_len;
                        uint8_t RRAttribute_ver;

                        bytesused += rockridge_bytestoskip;
                        //      printf("entry %s namelen %d bytesused: %u \n",direntryptr->name,namelen,bytesused);
                        //      dump_hex(&ptr[bytesused],len-bytesused);

                        do
                        {
                            RRAttribute_ptr = &ptr[bytesused];
                            RRAttribute_len = RRAttribute_ptr[2] - 4;
                            RRAttribute_ver = RRAttribute_ptr[3];

                            //       printf("Signature: %c%c len(payload): %d ver: %d datalen: %d\n",RRAttribute_ptr[0],RRAttribute_ptr[1],RRAttribute_ptr[2],RRAttribute_ptr[3],RRAttribute_len);
                            //       dump_hex(RRAttribute_ptr,RRAttribute_len+4);

                            if (RRAttribute_ver == 1)
                            {

                                if ((RRAttribute_ptr[0] == 'N') && (RRAttribute_ptr[1] == 'M')) //Name attribute
                                {
                                    namelen = RRAttribute_len - 1;
                                    memcpy(direntryptr->name, &RRAttribute_ptr[5], namelen);
                                    direntryptr->name[namelen] = 0;
                                }
                            }

                            bytesused += RRAttribute_ptr[2];
                        }

                        while ((bytesused < len) && (RRAttribute_ptr[2] > 0));
                    }
                }

                if (namelen > 2)
                {
                    if ((direntryptr->name[namelen - 2] == ';') && (direntryptr->name[namelen - 1] == '1'))direntryptr->name[namelen - 2] = 0;

                    if (direntryptr->name[namelen - 1] == ';')direntryptr->name[namelen - 1] = 0;
                }

                if (!direntryptr_prev) //first item in chain
                {

                    if (parent)parent->child = direntryptr;
                    else direntriesptr = direntryptr;
                }
                else
                {
                    direntryptr_prev->next = direntryptr;
                }

                //    debug_dump_direntry(level,direntryptr);

                if ((direntryptr->isdir) && (!avoid_recursion))
                    DVD_ScanContent_recurse(jolietmode, direntryptr, level + 1, direntryptr->firstsector, direntryptr->size_bytes, fake_inodecounter);
            }

            pos += len;
            direntryptr_prev = direntryptr;
        }

        while ((len != 0) && (pos < 2048));
    }

    free(localsectbuf);
    return 0;
}

static int DVD_ScanContent()
{
    int retval, currsect, done;
    uint32_t rootsect = 0;
    uint32_t rootdirlength = 0;
    int fake_inodecounter = 1;
    int fstype = -1; //0:standard ISO9660, 1:joliet
    uint8_t *sectbuf;
    uint8_t maxsectorstotry = 100;

    if (!WIIDVD_DiscPresent()) return -1;

    sectbuf = (uint8_t *)memalign(32, 0x800);

    if (!sectbuf) return -2;

    currsect = 16;

    done = 0;

    do
    {

        //read first volume descriptor
        retval = ReadSectorFromCache(sectbuf, currsect);
        //  retval=WIIDVD_ReadDVD(sectbuf,1,currsect);

        if (retval != 0)
        {
            return -1;
        }

        //  printf("Descriptor @ sect %d:\n",currsect);
        //  dump_hex(sectbuf,7);

        if ((sectbuf[1] == 67) && (sectbuf[2] == 68) && (sectbuf[3] == 48) && (sectbuf[4] == 48) && (sectbuf[5] == 49) && (sectbuf[6] == 1)) //volume descriptor signature
        {

            if (sectbuf[0] == 255)done = 1; //end marker;

            if ((sectbuf[0] == 0x02) && (fstype <= 1)) //Supplementart Volume Descriptor
            {
                //    printf("Supplementary Volume Descriptor found @ sect %d\n",currsect);
                //    printf("  Escape sequence:");
                //    dump_hex(&sectbuf[88],32);
                //    printf("\n");
                totalsectors = bothendian32_to_uint32(&sectbuf[80]);
                //    printf("Total sectors: %u\n",totalsectors);

                rootsect = bothendian32_to_uint32(&sectbuf[158]);
                //    printf("Root Sector: %d\n",rootsect);
                rootdirlength = bothendian32_to_uint32(&sectbuf[166]);
                //    printf("Root Length: %d\n",rootdirlength);

                if ((sectbuf[88] == 0x25) && (sectbuf[89] == 0x2F) && ((sectbuf[90] == 0x40) || (sectbuf[90] == 0x43) || (sectbuf[90] == 0x45)))
                {
                    //     if(sectbuf[90]==0x40)printf("  joliet Level1 descriptor found\n");
                    //     if(sectbuf[90]==0x43)printf("  joliet Level2 descriptor found\n");
                    //     if(sectbuf[90]==0x45)printf("  joliet Level3 descriptor found\n");
                    fstype = 1; //joliet
                }

            }

            if ((sectbuf[0] == 0x01) && (fstype <= 0)) //Primary Volume Descriptor
            {
                //    printf("Primary Volume Descriptor found @ sect %d\n",currsect);
                totalsectors = bothendian32_to_uint32(&sectbuf[80]);
                //    printf("Total sectors: %u\n",totalsectors);

                rootsect = bothendian32_to_uint32(&sectbuf[158]);
                //    printf("Root Sector: %d\n",rootsect);
                rootdirlength = bothendian32_to_uint32(&sectbuf[166]);
                //    printf("Root Length: %d\n",rootdirlength);
                fstype = 0;
            }

        }
        currsect++;
        maxsectorstotry--;
    }

    while ((!done) && (maxsectorstotry));

    if (fstype >= 0)
    {

        struct DIRENTRY *direntryptr;

        if (direntriesptr)freedirentrieslist();

        direntryptr = (struct DIRENTRY *)malloc(sizeof(struct DIRENTRY));

        if (direntryptr == NULL) return -1;

        totalentries++;

        memset(direntryptr, 0, sizeof(struct DIRENTRY));

        direntryptr->fake_inode = fake_inodecounter++;

        direntryptr->firstsector = rootsect;

        direntryptr->size_bytes = rootdirlength;

        direntryptr->size_sectors = ((direntryptr->size_bytes) - 1) >> 11;

        direntryptr->next = NULL;

        direntryptr->child = NULL;

        direntryptr->name[0] = 0;

        direntryptr->isfile = 0;

        direntryptr->isdir = 1;

        direntriesptr = direntryptr;

        return DVD_ScanContent_recurse(fstype == 1 ? 1 : 0, direntryptr, 0, rootsect, rootdirlength, &fake_inodecounter);

        //  debug_dump_tree();
    }

    free(sectbuf);
    return 0;
}

#ifndef DEBUG
const devoptab_t dotab_dvd =
    {
        "dvd",

        sizeof (struct FILESTATE),
        _DVD_open_r,
        _DVD_close_r,
        NULL,
        _DVD_read_r,
        _DVD_seek_r,
        _DVD_fstat_r,
        _DVD_stat_r,
        NULL,
        NULL,
        _DVD_chdir_r,
        NULL,
        NULL,

        sizeof (struct DIR_STATE),
        _DVD_diropen_r,
        _DVD_dirreset_r,
        _DVD_dirnext_r,
        _DVD_dirclose_r,
        _DVD_statvfs_r,
        NULL,                // device ftruncate_r
        NULL,            // device fsync_r
        NULL        /* Device data */

    };


static void dotab_dvd_add(void)
{
    AddDevice(&dotab_dvd);
}

#endif


#ifdef DEBUG
int main(int argc, char *argv[])
{
    printf("PC DEBUG MODE\n");
    WIIDVD_Init();
    WIIDVD_Mount();
    debug_dump_tree();
    WIIDVD_Close();
    return 0;
}

#endif

///////////////////////////////////////////
//         CACHE FUNCTIONS              //
///////////////////////////////////////////


static inline void _DVD_lock()
{
    LWP_MutexLock(_DVD_mutex);
}

static inline void _DVD_unlock()
{
    LWP_MutexUnlock(_DVD_mutex);
}

static void DestroyReadAheadCache()
{
    int i;

    if (ReadAheadCache == NULL)
    {
        RA_pages = 0;
        RA_sectors = 0;
        return ;
    }

    for (i = 0; i < RA_pages; i++)
    {
        if (ReadAheadCache[i].ptr != NULL)
            free(ReadAheadCache[i].ptr);
    }

    free(ReadAheadCache);
    ReadAheadCache = NULL;
    RA_pages = 0;
    RA_sectors = 0;
}

static void DVDEnableReadAhead(uint32_t pages, uint32_t sectors)
{
    int i, j;

    DestroyReadAheadCache();

    if (pages == 0 || sectors == 0) return ;

    if (sectors > 26)sectors = 26;

    RA_pages = pages;

    RA_sectors = sectors;

    ReadAheadCache = (cache_page *)malloc(sizeof(cache_page) * RA_pages);

    if (ReadAheadCache == NULL) return ;

    for (i = 0; i < RA_pages; i++)
    {
        ReadAheadCache[i].sector = CACHE_FREE;
        ReadAheadCache[i].last_used = 0;
        ReadAheadCache[i].ptr = memalign(32, SECTOR_SIZE * RA_sectors);

        if (ReadAheadCache[i].ptr == NULL)
        {
            for (j = i - 1;j >= 0;j--)
                if (ReadAheadCache[j].ptr)free(ReadAheadCache[j].ptr);

            free(ReadAheadCache);

            ReadAheadCache = NULL;

            return ;
        }

        memset(ReadAheadCache[i].ptr, 0, SECTOR_SIZE * RA_sectors);
    }
}

static int ReadSectorFromCache(void *buf, uint32_t sector)
{
    int retval;
    int i, leastUsed;

    if (ReadAheadCache == NULL) return WIIDVD_ReadDVD(buf, 1, sector);

    _DVD_lock();

    leastUsed = 0;

    for (i = 0; i < RA_pages; i++)
    {
        if ( (sector >= ReadAheadCache[i].sector) && (sector < (ReadAheadCache[i].sector + RA_sectors)) )
        {
            ReadAheadCache[i].last_used = gettick();
            memcpy(buf, ReadAheadCache[i].ptr + ((sector - ReadAheadCache[i].sector) * SECTOR_SIZE), SECTOR_SIZE);
            _DVD_unlock();
            return 0;
        }
    }


    for (i = 1; i < RA_pages; i++)
    {
        if ((ReadAheadCache[i].last_used < ReadAheadCache[leastUsed].last_used))
            leastUsed = i;
    }

    retval = WIIDVD_ReadDVD(ReadAheadCache[leastUsed].ptr, RA_sectors, sector);

    if (retval)
    {
        ReadAheadCache[leastUsed].sector = CACHE_FREE;
        ReadAheadCache[leastUsed].last_used = 0;
        _DVD_unlock();
        return retval;
    }

    ReadAheadCache[leastUsed].sector = sector;
    ReadAheadCache[leastUsed].last_used = gettick();
    memcpy(buf, ReadAheadCache[leastUsed].ptr, SECTOR_SIZE);
    _DVD_unlock();

    return 0;
}
