#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/iosupport.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <ogcsys.h>
#include <new>
#include "ramdisk.h"

#define NAMELENMAX	0x80
#define MAXPATHLEN	0x100

#define FPRINTF(f, ...) {FILE*_fp=fopen("SD:/log.log", "a"); if(_fp) { fprintf(_fp, "%s(%i):" f "\n", __FILE__, __LINE__, ##__VA_ARGS__); fclose(_fp);}}



class RAMDISK_PARTITION;
class RAMDISK_LINK_ENTRY;
class RAMDISK_DIR_ENTRY;
class RAMDISK_FILE_ENTRY;
class RAMDISK_BASE_ENTRY
{
public:
	RAMDISK_BASE_ENTRY(const char *Name, RAMDISK_DIR_ENTRY *Parent);
	virtual ~RAMDISK_BASE_ENTRY();
	void Rename(const char *Name);
	RAMDISK_LINK_ENTRY *IsLink();
	RAMDISK_DIR_ENTRY *IsDir();
	RAMDISK_FILE_ENTRY *IsFile();
	RAMDISK_PARTITION *GetPartition();
	char *name;
	RAMDISK_DIR_ENTRY *parent;
	RAMDISK_BASE_ENTRY *next;
};
class RAMDISK_LINK_ENTRY : public RAMDISK_BASE_ENTRY
{
public:
	RAMDISK_LINK_ENTRY(const char* Name, RAMDISK_DIR_ENTRY *Parent, RAMDISK_BASE_ENTRY *Link);
	RAMDISK_BASE_ENTRY *link;
};
class RAMDISK_DIR_ENTRY : public RAMDISK_BASE_ENTRY
{
public:
	RAMDISK_DIR_ENTRY(const char* Name, RAMDISK_DIR_ENTRY *Parent);
	~RAMDISK_DIR_ENTRY();
	RAMDISK_BASE_ENTRY *FindEntry(const char* Path);
	RAMDISK_FILE_ENTRY *CreateFile(const char *Filename);
	RAMDISK_DIR_ENTRY *CreateDir(const char *Filename);
	void RemoveEntry(RAMDISK_BASE_ENTRY *Entry);
	RAMDISK_BASE_ENTRY *first;
	RAMDISK_BASE_ENTRY *last;
};
typedef struct
{
	RAMDISK_DIR_ENTRY *dir;
	RAMDISK_BASE_ENTRY *current_entry;
} DIR_STRUCT;


class RAMDISK_PARTITION : public RAMDISK_DIR_ENTRY
{
public:
	RAMDISK_PARTITION(const char *Mountpoint, bool AutoMount);
	RAMDISK_BASE_ENTRY *FindEntry(const char* Path);
	RAMDISK_DIR_ENTRY *FindPath(const char* Path, const char **Basename=NULL);
	RAMDISK_DIR_ENTRY *cwd;
	bool automount;
};

class FILE_DATA
{
public:
	FILE_DATA(size_t Len);
	~FILE_DATA();
	u8 *data;
	size_t len;
	FILE_DATA *next;
};
typedef struct
{
	RAMDISK_FILE_ENTRY *file;
	bool isLink;
	u32 current_pos;
	bool read;
	bool write;
} FILE_STRUCT;
class RAMDISK_FILE_ENTRY : public RAMDISK_BASE_ENTRY
{
public:
	RAMDISK_FILE_ENTRY(const char* Name, RAMDISK_DIR_ENTRY *Parent);
	~RAMDISK_FILE_ENTRY();
	bool Truncate(size_t newLen);
	bool AddCluster(size_t size);
	size_t Read(struct _reent *r, FILE_STRUCT *fileStruct, char *ptr, size_t len);
	size_t Write(struct _reent *r, FILE_STRUCT *fileStruct, const char *ptr, size_t len);
	size_t		file_len;
	size_t		cluster_len;
	FILE_DATA *first_cluster;
	FILE_DATA *last_cluster;
};

RAMDISK_LINK_ENTRY::RAMDISK_LINK_ENTRY(const char* Name, RAMDISK_DIR_ENTRY *Parent, RAMDISK_BASE_ENTRY *Link)
:
RAMDISK_BASE_ENTRY(Name, Parent),
link(Link)
{

}


RAMDISK_BASE_ENTRY::RAMDISK_BASE_ENTRY(const char *Name, RAMDISK_DIR_ENTRY *Parent)
:
name(strdup(Name)),
parent(Parent),
next(NULL)
{}
RAMDISK_BASE_ENTRY::~RAMDISK_BASE_ENTRY()
{
	free(name);
	if(parent) parent->RemoveEntry(this);
}
void RAMDISK_BASE_ENTRY::Rename(const char *Name)
{
	free(name);
	name = strdup(Name);
}

inline RAMDISK_LINK_ENTRY *RAMDISK_BASE_ENTRY::IsLink() 
{
	return dynamic_cast<RAMDISK_LINK_ENTRY*>(this);
}
inline RAMDISK_DIR_ENTRY *RAMDISK_BASE_ENTRY::IsDir()
{
	RAMDISK_LINK_ENTRY *lentry = dynamic_cast<RAMDISK_LINK_ENTRY*>(this);
	return dynamic_cast<RAMDISK_DIR_ENTRY*>(lentry ? lentry->link : this);
}
inline RAMDISK_FILE_ENTRY *RAMDISK_BASE_ENTRY::IsFile() 
{
	RAMDISK_LINK_ENTRY *lentry = dynamic_cast<RAMDISK_LINK_ENTRY*>(this);
	return dynamic_cast<RAMDISK_FILE_ENTRY*>(lentry ? lentry->link : this);
}
RAMDISK_PARTITION *RAMDISK_BASE_ENTRY::GetPartition()
{
	for(RAMDISK_BASE_ENTRY *entry = this; entry; entry = entry->parent)
		if(entry->parent == NULL)
			return dynamic_cast<RAMDISK_PARTITION *>(entry);
	return NULL;
}


RAMDISK_DIR_ENTRY::RAMDISK_DIR_ENTRY(const char* Name, RAMDISK_DIR_ENTRY *Parent)
:
RAMDISK_BASE_ENTRY(Name, Parent)
{
	RAMDISK_BASE_ENTRY* entry = new RAMDISK_LINK_ENTRY(".", this, this);
	first = entry;
	last = entry;
	if(parent)
	{
		entry = new RAMDISK_LINK_ENTRY("..", this, parent);
		first->next = entry;
		last = entry;
	}
}
RAMDISK_DIR_ENTRY::~RAMDISK_DIR_ENTRY()
{
	while(first)
	{
		first->parent = NULL; // ~RAMDISK_BASE_ENTRY() no calls RemoveEntry()
		RAMDISK_BASE_ENTRY* next = first->next;
		delete first;
		first = next;
	}
}

RAMDISK_BASE_ENTRY *RAMDISK_DIR_ENTRY::FindEntry(const char* Path)
{
	const char* dirpath = Path;
	const char* cptr;
	while( dirpath[0] == '/') dirpath++;	// move past leading '/'

	if(dirpath[0] == '\0')						// this path is found
		return this;

	cptr = strchr(dirpath,'/');				// find next '/'
	if(cptr == NULL)
		cptr = strchr(dirpath,'\0');			// cptr at end
	for(RAMDISK_BASE_ENTRY *curr = first; curr; curr=curr->next)
	{
		if( strncmp(curr->name, dirpath, cptr-dirpath) == 0)
		{
			if(RAMDISK_DIR_ENTRY *dir = curr->IsDir())
				return dir->FindEntry(cptr);
			else
				return curr;
		}
	}
	
	return NULL;
}
RAMDISK_FILE_ENTRY *RAMDISK_DIR_ENTRY::CreateFile(const char *Filename)
{
	try
	{
		RAMDISK_FILE_ENTRY* file = new RAMDISK_FILE_ENTRY(Filename, this);
	if(!first) first = file;
	last->next = file;
	last = file;
	return file;
}
catch(...) { return NULL; }
}
RAMDISK_DIR_ENTRY *RAMDISK_DIR_ENTRY::CreateDir(const char *Filename)
{
	try
	{
	RAMDISK_DIR_ENTRY* dir = new RAMDISK_DIR_ENTRY(Filename, this);
	if(!first) first = dir;
	last->next = dir;
	last = dir;
	return dir;
}
catch(...) { return NULL; }
}
void RAMDISK_DIR_ENTRY::RemoveEntry(RAMDISK_BASE_ENTRY *Entry)
{
	RAMDISK_BASE_ENTRY **p_last = NULL;
	for(RAMDISK_BASE_ENTRY **curr = &first; *curr; curr=&((*curr)->next) )
	{
		if( *curr == Entry )
		{
			*curr = Entry->next;
			if(Entry->next == NULL) // entry is last
				last = *p_last; 
			break;
		}
		p_last = curr;
	}
}

RAMDISK_PARTITION::RAMDISK_PARTITION(const char *Mountpoint, bool AutoMount) :
RAMDISK_DIR_ENTRY(Mountpoint, NULL),
cwd(this),
automount(AutoMount)
{
}

RAMDISK_BASE_ENTRY * RAMDISK_PARTITION::FindEntry(const char* path)
{
	char *cptr;
	if ( (cptr=strchr(path,':')) )
		path=cptr+1;			//move path past any device names
	if ( strchr(path, ':') != NULL )
	{
//		r->_errno = EINVAL;
		return NULL;
	}
	if(*path=='/')													// if first character is '/' use absolute root path
		return RAMDISK_DIR_ENTRY::FindEntry(path);
	else																// else use current working dir
		return cwd->FindEntry(path);
}
RAMDISK_DIR_ENTRY * RAMDISK_PARTITION::FindPath(const char* path, const char **basename)
{
	int pathLen = strlen(path);
	char dirfilename[pathLen+1]; 					// to hold a full path 
	const char *cptr = path+pathLen;				// find the end...
	const char *filename = NULL;					// to hold filename
	while(cptr-->path)								//search till start
	{
		if((*cptr=='/') || (*cptr==':'))				// split at either / or : (whichever comes first form the end!)
		{
			cptr++;
			strlcpy(dirfilename, path, 1+cptr-path);	//copy string up till and including / or :
			filename = cptr;									//filename = now remainder of string
			break;
		}
	} 
	
	if(!filename)
	{
		filename		= path;				//filename = complete path
		dirfilename[0]	= 0;				//make directory path ""
	}
	RAMDISK_BASE_ENTRY *entry = FindEntry(dirfilename);
	if(entry)
	{
		if(basename) *basename = filename;
		return entry->IsDir();
	}
	return NULL;
}


FILE_DATA::FILE_DATA(size_t Len) :
next(NULL)
{
	data = new u8[Len];
	len = Len;
	memset(data, 0, len);
}	
FILE_DATA::~FILE_DATA()
{
	delete [] data;
	delete next;
}


RAMDISK_FILE_ENTRY::RAMDISK_FILE_ENTRY(const char* Name, RAMDISK_DIR_ENTRY *Parent)
:
RAMDISK_BASE_ENTRY(Name, Parent),
file_len(0),
cluster_len(0),
first_cluster(NULL),
last_cluster(NULL)
{}
RAMDISK_FILE_ENTRY::~RAMDISK_FILE_ENTRY()
{
	Truncate(0);
}
#define CLUSTER_SIZE 4*1024

bool RAMDISK_FILE_ENTRY::Truncate(size_t newSize)
{
	if (newSize > cluster_len)
	{
		// Expanding the file over cluster_len
		return AddCluster(newSize - cluster_len);
	}
	else if (newSize < file_len)
	{
		// Shrinking the file
		FILE_DATA *prev_cluster = NULL;
		size_t len = 0;
		for(FILE_DATA **p_cluster = &first_cluster; *p_cluster; p_cluster = &(*p_cluster)->next)
		{
			if(len >= newSize)
			{
				last_cluster = prev_cluster;
				delete *p_cluster;
				(*p_cluster) = NULL;
				break;
			}
			len += (*p_cluster)->len;
			prev_cluster = *p_cluster;
		}
	}
	file_len = newSize;
	return true;
}



bool RAMDISK_FILE_ENTRY::AddCluster(size_t len)
{
	if(len < CLUSTER_SIZE)
		len = CLUSTER_SIZE;
	try
	{
		*(last_cluster ? &last_cluster->next : &first_cluster) = last_cluster = new FILE_DATA(len); 
		cluster_len += len;
		return true;
	}
	catch(...) { return false; }
}

size_t RAMDISK_FILE_ENTRY::Read(struct _reent *r, FILE_STRUCT *fileStruct, char *ptr, size_t len)
{
	if (!fileStruct->read)
	{
		r->_errno = EBADF;
		return 0;
	} 
	// Short circuit cases where len is 0 (or less)
	if (len <= 0)
		return 0;
	// Don't try to read if the read pointer is past the end of file
	if (fileStruct->current_pos >= file_len)
	{
		r->_errno = EOVERFLOW;
		return 0;
	}

	// Don't read past end of file
	if (len + fileStruct->current_pos > file_len)
	{
		r->_errno = EOVERFLOW;
		len = fileStruct->current_pos - file_len;
	}

	off_t pos = fileStruct->current_pos;
	size_t readed = 0;
	size_t max_len = file_len;
	for(FILE_DATA *cluster = first_cluster; cluster; cluster = cluster->next)
	{
		if(pos > cluster->len)
		{
			pos		-= cluster->len;
			max_len	-= cluster->len;
		}
		else
		{
			size_t read = max_len;
			if(read > cluster->len)
				read = cluster->len;
			read -= pos;
			if(read > len)
				read = len;
			memcpy(ptr, &(cluster->data[pos]), read);
			readed += read;
			ptr += read;
			len -= read;
			if(len == 0)
				break;
			pos		-= cluster->len;
			max_len	-= cluster->len;
			
		}
	}
	fileStruct->current_pos += readed;
	return readed;
}
size_t RAMDISK_FILE_ENTRY::Write(struct _reent *r, FILE_STRUCT *fileStruct, const char *ptr, size_t len)
{

	if (!fileStruct->write)
	{
		r->_errno = EBADF;
		return 0;
	} 
	// Short circuit cases where len is 0 (or less)
	if (len <= 0)
		return 0;

	off_t pos = fileStruct->current_pos;
	if(cluster_len < (pos+len) && !AddCluster((pos+len) - cluster_len))
	{
		// Couldn't get a cluster, so abort
		r->_errno = ENOSPC;
		return 0;
	}
	if(file_len < (pos+len))
		file_len = (pos+len);
	
	size_t written = 0;
	size_t max_len = cluster_len;
	for(FILE_DATA *cluster = first_cluster; cluster; cluster = cluster->next)
	{
		if(pos > cluster->len)
		{
			pos		-= cluster->len;
			max_len	-= cluster->len;
		}
		else
		{
			size_t write = cluster->len - pos;
			if(write > len) write = len;
			memcpy(&(cluster->data[pos]), ptr, write);
			written += write;
			ptr += write;
			len -= write;
			if(len == 0)
				break;
			pos		-= cluster->len;
			max_len	-= cluster->len;
		}
	}
	fileStruct->current_pos += written;
	return written;
}


static int ramdiskFS_open_r(struct _reent *r, void *fileStruct, const char *path, int flags, int mode);
static int ramdiskFS_close_r(struct _reent *r, int fd);
static int ramdiskFS_write_r(struct _reent *r, int fd, const char *ptr, size_t len);
static int ramdiskFS_read_r(struct _reent *r, int fd, char *ptr, size_t len);
static off_t ramdiskFS_seek_r(struct _reent *r, int fd, off_t pos, int dir);
static int ramdiskFS_fstat_r(struct _reent *r, int fd, struct stat *st);
static int ramdiskFS_stat_r(struct _reent *r, const char *file, struct stat *st);
static int ramdiskFS_unlink_r(struct _reent *r, const char *name);
static int ramdiskFS_chdir_r(struct _reent *r, const char *name);
static int ramdiskFS_mkdir_r(struct _reent *r, const char *path, int mode);

static DIR_ITER* ramdiskFS_diropen_r(struct _reent *r, DIR_ITER *dirState, const char *path);
static int ramdiskFS_dirreset_r(struct _reent *r, DIR_ITER *dirState);
static int ramdiskFS_dirnext_r(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *st);
static int ramdiskFS_dirclose_r(struct _reent *r, DIR_ITER *dirState);

static int ramdiskFS_ftruncate_r(struct _reent *r, int fd, off_t len);


static devoptab_t ramdiskFS_devoptab={
	"ramdisk",
	sizeof(FILE_STRUCT),			//	int	structSize;
	&ramdiskFS_open_r,				//	int (*open_r)(struct _reent *r, void *fileStruct, const char *path,int
									//		flags,int mode);
	&ramdiskFS_close_r,				//	int (*close_r)(struct _reent *r, int fd);
	&ramdiskFS_write_r,				//	int (*write_r)(struct _reent *r, int fd, const char *ptr, int len);
	&ramdiskFS_read_r,				//	int (*read_r)(struct _reent *r, int fd, char *ptr, int len);
	&ramdiskFS_seek_r,				//	off_t (*seek_r)(struct _reent *r, off_t fd, int pos, int dir);
	&ramdiskFS_fstat_r,				//	int (*fstat_r)(struct _reent *r, int fd, struct stat *st);
	&ramdiskFS_stat_r,				//	int (*stat_r)(struct _reent *r, const char *file, struct stat *st);
	NULL,							//	int (*link_r)(struct _reent *r, const char *existing, const char  *newLink);
	&ramdiskFS_unlink_r,				//	int (*unlink_r)(struct _reent *r, const char *name);
	&ramdiskFS_chdir_r,				//	int (*chdir_r)(struct _reent *r, const char *name);
	NULL,							//	int (*rename_r) (struct _reent *r, const char *oldName, const char *newName);
	ramdiskFS_mkdir_r,				//	int (*mkdir_r) (struct _reent *r, const char *path, int mode);
	sizeof(DIR_STRUCT),				//	int dirStateSize;
	&ramdiskFS_diropen_r,				//	DIR_ITER* (*diropen_r)(struct _reent *r, DIR_ITER *dirState, const char *path);
	&ramdiskFS_dirreset_r,			//	int (*dirreset_r)(struct _reent *r, DIR_ITER *dirState);
	&ramdiskFS_dirnext_r,				//	int (*dirnext_r)(struct _reent *r, DIR_ITER *dirState, char *filename,
									//		struct stat *filestat);
	&ramdiskFS_dirclose_r,			//	int (*dirclose_r)(struct _reent *r, DIR_ITER *dirState);
	NULL,							//	statvfs_r
	&ramdiskFS_ftruncate_r,				//	int (*ftruncate_r)(struct _reent *r, int fd, off_t len);

	NULL,							//	fsync_r,
	NULL							//	Device data
};

//---------------------------------------------------------------------------------
static inline RAMDISK_PARTITION* ramdiskFS_getPartitionFromPath(const char* path) {
//---------------------------------------------------------------------------------
	const devoptab_t *devops = GetDeviceOpTab(path);
	if (!devops)
		return NULL;
	return *((RAMDISK_PARTITION**)devops->deviceData);
}

//---------------------------------------------------------------------------------
// File functions
//---------------------------------------------------------------------------------
static int ramdiskFS_open_r(struct _reent *r, void *file_Struct, const char *path, int flags, int mode) {
//---------------------------------------------------------------------------------
FPRINTF("ramdiskFS_open_r(%s)", path); 
	FILE_STRUCT *fileStruct = (FILE_STRUCT*)file_Struct;


	RAMDISK_PARTITION *partition = ramdiskFS_getPartitionFromPath(path);
	if ( partition == NULL )
	{
		r->_errno = ENODEV;
		return -1;
	}
FPRINTF("ramdiskFS_open_r Partition found"); 

	if ((flags & 0x03) == O_RDONLY) {
		// Open the file for read-only access
		fileStruct->read = true;
		fileStruct->write = false;
	} else if ((flags & 0x03) == O_WRONLY) {
		// Open file for write only access
		fileStruct->read = false;
		fileStruct->write = true;
	} else if ((flags & 0x03) == O_RDWR) {
		// Open file for read/write access
		fileStruct->read = true;
		fileStruct->write = true;
	} else {
		r->_errno = EACCES;
		return -1;
	}
	RAMDISK_BASE_ENTRY *entry = partition->FindEntry(path);
	
	// The file shouldn't exist if we are trying to create it
	if (entry && (flags & O_CREAT) && (flags & O_EXCL)) 
	{
		r->_errno = EEXIST;
		return -1;
	}
FPRINTF("ok"); 
	// It should not be a directory if we're openning a file,
	if (entry && entry->IsDir())
	{
		r->_errno = EISDIR;
		return -1;
	}
FPRINTF("ok"); 

	fileStruct->isLink = entry ? entry->IsLink() : false;
	fileStruct->file = entry ? entry->IsFile() : NULL;
	if(!fileStruct->file)					// entry not exists
	{
		if (flags & O_CREAT)
		{
			const char *filename;
			RAMDISK_DIR_ENTRY *dir = partition->FindPath(path, &filename);
			if(!dir)
			{
				r->_errno = ENOTDIR;
				return -1;
			}
			fileStruct->file = dir->CreateFile(filename);
		}
		else
		{
			// file doesn't exist, and we aren't creating it
			r->_errno = ENOENT;
			return -1;
		}
	}
FPRINTF("ok"); 
	if(fileStruct->file)
	{
		fileStruct->current_pos = 0;
		// Truncate the file if requested
		if ((flags & O_TRUNC) && fileStruct->write) 
			fileStruct->file->Truncate(0);
		if (flags & O_APPEND)
			fileStruct->current_pos = fileStruct->file->file_len;
		return 0;
FPRINTF("ramdiskFS_open_r ok"); 
	}
	r->_errno = ENOENT;
	return(-1);
}

//---------------------------------------------------------------------------------
static int ramdiskFS_close_r(struct _reent *r, int fd) {
//---------------------------------------------------------------------------------
	return(0);
}

//---------------------------------------------------------------------------------
static int ramdiskFS_read_r(struct _reent *r, int fd, char *ptr, size_t len) {
//---------------------------------------------------------------------------------
	FILE_STRUCT *fileStruct = (FILE_STRUCT*)fd;
	return fileStruct->file->Read(r, fileStruct, ptr, len);
}

//---------------------------------------------------------------------------------
static int ramdiskFS_write_r(struct _reent *r, int fd, const char *ptr, size_t len) {
//---------------------------------------------------------------------------------
	FILE_STRUCT *fileStruct = (FILE_STRUCT*)fd;
	return fileStruct->file->Write(r, fileStruct, ptr, len);
}

//---------------------------------------------------------------------------------
static off_t ramdiskFS_seek_r(struct _reent *r, int fd, off_t pos, int dir) {
//---------------------------------------------------------------------------------
	//need check for eof here...
	FILE_STRUCT *fileStruct = (FILE_STRUCT*)fd;
FPRINTF("ramdiskFS_seek_r(%s)", fileStruct->file->name); 
	
	switch(dir)
	{
		case SEEK_SET:
			break;
		case SEEK_CUR:
			pos += fileStruct->current_pos;
			break;
		case SEEK_END:
			pos += fileStruct->file->file_len;		// set start to end of file
			break;
		default:
			r->_errno = EINVAL;
			return -1;
	}
	return fileStruct->current_pos = pos;
}

//---------------------------------------------------------------------------------
static int ramdiskFS_fstat_r(struct _reent *r, int fd, struct stat *st) {
//---------------------------------------------------------------------------------
FPRINTF("ramdiskFS_fstat_r"); 
	FILE_STRUCT *fileStruct = (FILE_STRUCT*)fd;

	st->st_mode = fileStruct->isLink ? S_IFLNK : S_IFREG;
	st->st_size = fileStruct->file->file_len;
	return(0);
}

//---------------------------------------------------------------------------------
static int ramdiskFS_stat_r(struct _reent *r, const char *file, struct stat *st) {
//---------------------------------------------------------------------------------
FPRINTF("ramdiskFS_stat_r(%s)", file); 
	FILE_STRUCT fileStruct;
	DIR_STRUCT dirStruct;
	DIR_ITER dirState;
	dirState.dirStruct=&dirStruct;		//create a temp dirstruct
	int ret;

	if( ramdiskFS_open_r(r, &fileStruct, file, 0, 0) ==0 )
	{
		ret = ramdiskFS_fstat_r(r, (int)&fileStruct, st);
		ramdiskFS_close_r(r, (int)&fileStruct);
		return(ret);
	}
	else if( (ramdiskFS_diropen_r(r, &dirState, file)!=NULL) ) 
	{
		st->st_mode = S_IFDIR;
		ramdiskFS_dirclose_r(r, &dirState);
		return(0);
	}
	r->_errno = ENOENT;
	return(-1);
}

//---------------------------------------------------------------------------------
static int ramdiskFS_unlink_r(struct _reent *r, const char *name) {
//---------------------------------------------------------------------------------
FPRINTF("ramdiskFS_unlink_r(%s)", name); 
	RAMDISK_PARTITION *partition = ramdiskFS_getPartitionFromPath(name);
	if ( partition == NULL )
	{
		r->_errno = ENODEV;
		return -1;
	}
	RAMDISK_BASE_ENTRY *entry = partition->FindEntry(name);
	if(!entry)
	{
		r->_errno = ENOENT;
		return -1;
	}
	
	if (entry->IsLink())
	{
		if(entry->name[0] == '.' && (entry->name[1] == '\0' || (entry->name[1] == '.' && entry->name[2] == '\0')))
		{
			r->_errno = EPERM;
			return -1;
		}
		delete entry;
		return 0;
	}

	if (RAMDISK_DIR_ENTRY *dir = entry->IsDir())
	{
		for(RAMDISK_BASE_ENTRY *entry = dir->first; entry; entry = entry->next)
		{
			if(!(entry->name[0] == '.' && (entry->name[1] == '\0' 
						|| (entry->name[1] == '.' && entry->name[2] == '\0'))))
			{
				r->_errno = EPERM;
				return -1;
			}
		}
	}
	delete entry;
	return 0;
}

//---------------------------------------------------------------------------------
static int ramdiskFS_chdir_r(struct _reent *r, const char *name) {
//---------------------------------------------------------------------------------
FPRINTF("ramdiskFS_chdir_r(%s)", name); 
	DIR_STRUCT dirStruct;
	DIR_ITER dirState;
	dirState.dirStruct=&dirStruct;
	if( (name == NULL) )
	{
		r->_errno = ENODEV;
		return -1;
	}
	
	if( (ramdiskFS_diropen_r(r, &dirState, name) == NULL) )
		return -1;

	RAMDISK_PARTITION *partition = dirStruct.dir->GetPartition();
	if ( partition == NULL )
	{
		r->_errno = ENODEV;
		return -1;
	}
	partition->cwd = dirStruct.dir;
	ramdiskFS_dirclose_r(r, &dirState);
FPRINTF("ok"); 
	return 0;
}

//---------------------------------------------------------------------------------
static int ramdiskFS_mkdir_r(struct _reent *r, const char *path, int mode) {
//---------------------------------------------------------------------------------
FPRINTF("ramdiskFS_mkdir_r(%s)", path); 
	RAMDISK_PARTITION *partition = ramdiskFS_getPartitionFromPath(path);
	if ( partition == NULL )
	{
		r->_errno = ENODEV;
		return -1;
	}
	RAMDISK_BASE_ENTRY *entry = partition->FindEntry(path);
	if (entry) 
	{
		r->_errno = EEXIST;
		return -1;
	}
	const char *filename;
	RAMDISK_DIR_ENTRY *dir = partition->FindPath(path, &filename);
	if(!dir)
	{
		r->_errno = ENOTDIR;
		return -1;
	}
	dir->CreateDir(filename);
	return 0;
}

//---------------------------------------------------------------------------------
// Directory functions
//---------------------------------------------------------------------------------
static DIR_ITER* ramdiskFS_diropen_r(struct _reent *r, DIR_ITER *dirState, const char *path) {
//---------------------------------------------------------------------------------

	FPRINTF("DirOpen %s", path);
	DIR_STRUCT *dirStruct = (DIR_STRUCT*)dirState->dirStruct;
	char *cptr;

	RAMDISK_PARTITION *partition = ramdiskFS_getPartitionFromPath(path);
	FPRINTF("partition %p", partition);
	if ( partition == NULL )
	{
		r->_errno = ENODEV;
		return NULL;
	}

	if ( (cptr=strchr(path,':')) )
		path=cptr+1;			//move path past any device names
	if ( strchr(path, ':') != NULL )
	{
		r->_errno = EINVAL;
		return NULL;
	}

	if(*path=='/')						//if first character is '/' use absolute root path
		dirStruct->dir = partition;	//first root dir
	else
		dirStruct->dir = partition->cwd;	//else use current working dir

	RAMDISK_BASE_ENTRY *entry = dirStruct->dir->FindEntry(path);
	FPRINTF("entry %p", entry);
	if(entry==NULL)
	{
		r->_errno = ENOENT;
		return NULL;
	}
	dirStruct->dir = entry->IsDir();
	if(dirStruct->dir==NULL)
	{
		r->_errno = ENOTDIR;
		return NULL;
	}
	dirStruct->current_entry = dirStruct->dir->first;
	return dirState;
}

/*Consts containing relative system path strings*/
//reset dir to start of entry selected by dirStruct->cur_dir_id
//---------------------------------------------------------------------------------
static int ramdiskFS_dirreset_r(struct _reent *r, DIR_ITER *dirState) {
//---------------------------------------------------------------------------------
	DIR_STRUCT *dirStruct = (DIR_STRUCT*)dirState->dirStruct;
	dirStruct->current_entry = dirStruct->dir->first;
	return(0);
}

//---------------------------------------------------------------------------------
static int ramdiskFS_dirnext_r(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *st) {
//---------------------------------------------------------------------------------
	DIR_STRUCT *dirStruct = (DIR_STRUCT*)dirState->dirStruct;
//	RAMDISK_BASE_ENTRY **dirStruct = (RAMDISK_BASE_ENTRY**)dirState->dirStruct;

	FPRINTF("DirNext");
	
	if(dirStruct->current_entry) 
	{
	FPRINTF("current_entry = %s",dirStruct->current_entry->name);
		strcpy(filename, dirStruct->current_entry->name);
		if(dirStruct->current_entry->IsDir())
		{
	FPRINTF("IsDir");
			if(st) st->st_mode=S_IFDIR;
		}
		else
		{
	FPRINTF("IsFile");
			if(st) st->st_mode=0;
		}
		dirStruct->current_entry = dirStruct->current_entry->next;
		return(0);
	}
	r->_errno = ENOENT;
	return(-1);
}

//---------------------------------------------------------------------------------
static int ramdiskFS_dirclose_r(struct _reent *r, DIR_ITER *dirState) {
//---------------------------------------------------------------------------------
	return(0);
}

//---------------------------------------------------------------------------------
static int ramdiskFS_ftruncate_r(struct _reent *r, int fd, off_t len) {
//---------------------------------------------------------------------------------
	FILE_STRUCT *fileStruct = (FILE_STRUCT*)fd;

	if (len < 0)
	{
		// Trying to truncate to a negative size
		r->_errno = EINVAL;
		return -1;
	}
/*
	if ((sizeof(len) > 4) && len > (off_t)FILE_MAX_SIZE)
	{
		// Trying to extend the file beyond what supports
		r->_errno = EFBIG;
		return -1;
	} 
*/
	if (!fileStruct->write)
	{
		// Read-only file
		r->_errno = EINVAL;
		return -1;
	}
	if(fileStruct->file->Truncate(len))
		return 0;
	r->_errno = ENOSPC;
	return -1;
}

//---------------------------------------------------------------------------------
void ramdiskFS_Unmount(const char* mountpoint) {
//---------------------------------------------------------------------------------
	RAMDISK_PARTITION *partition;
	devoptab_t *devops = (devoptab_t*)GetDeviceOpTab(mountpoint);
	
	if (!devops)
		return;

	// Perform a quick check to make sure we're dealing with a ramdiskFS_ controlled device
	if (devops->open_r != ramdiskFS_devoptab.open_r)
		return;
	
	if (RemoveDevice (mountpoint) == -1)
		return;

	partition = *((RAMDISK_PARTITION **)devops->deviceData);
	if(partition->automount)
		delete partition;
	free (devops); 
}
extern "C" void ramdiskUnmount(const char *mountpoint) {
	ramdiskFS_Unmount(mountpoint);
}
//---------------------------------------------------------------------------------
int ramdiskFS_Mount(const char *mountpoint, void *handle) {
//---------------------------------------------------------------------------------
	FPRINTF("Mount %s", mountpoint);
	devoptab_t* devops;
	char* nameCopy;
	RAMDISK_PARTITION** partition;
	char Mountpoint[100];
	char *cptr;

	strlcpy(Mountpoint, mountpoint, sizeof(Mountpoint));
	int len = strlen(Mountpoint);
	cptr = strchr(Mountpoint, ':');
	if(cptr)
	{
		len = cptr-Mountpoint;
		*++cptr = 0;
	}
	else
		strlcat(Mountpoint, ":", sizeof(Mountpoint));
	ramdiskFS_Unmount(Mountpoint);
	if(handle) ramdiskFS_Unmount(((RAMDISK_PARTITION*)handle)->name);
	
	devops = (devoptab_t*)malloc(sizeof(devoptab_t) + sizeof(RAMDISK_PARTITION*) + len + 1);
	if (!devops)
		return false;

	partition = (RAMDISK_PARTITION**)(devops+1);	// Use the space allocated at the end of the devoptab struct
											// for storing the partition
	nameCopy = (char*)( partition+1);				// Use the space allocated at the end of the partition struct
											// for storing the name
	
	memcpy (devops, &ramdiskFS_devoptab, sizeof(ramdiskFS_devoptab)); // Add an entry for this device to the devoptab table

	strlcpy (nameCopy, Mountpoint, len + 1);
	devops->name = nameCopy;
	
	if(handle)
	{
		*partition = (RAMDISK_PARTITION*)handle;
		(*partition)->Rename(Mountpoint);
	}
	else
		*partition = new RAMDISK_PARTITION(Mountpoint, true);
	devops->deviceData			= partition; 
	FPRINTF("devops=%p nameCopy=%p(%s) partition=%p", devops, nameCopy, nameCopy, partition);

	if(AddDevice(devops)<0)
	{
		free(devops);
		return false;
	}
	FPRINTF("mounted");

	return true;

}
extern "C" int ramdiskMount(const char *mountpoint, void *handle) {
	return ramdiskFS_Mount(mountpoint, handle);
}
//---------------------------------------------------------------------------------
extern "C" void* ramdiskCreate() {
//---------------------------------------------------------------------------------
	return new RAMDISK_PARTITION("", false);
}
//---------------------------------------------------------------------------------
extern "C" void ramdiskDelete(void* Handle) {
//---------------------------------------------------------------------------------
	RAMDISK_PARTITION *partition = (RAMDISK_PARTITION*)Handle;
	if(partition->automount==false)
		ramdiskFS_Unmount(partition->name);
	delete partition;
}

