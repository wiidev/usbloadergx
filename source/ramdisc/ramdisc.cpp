#include <string.h>
#include "ramdisc.h"

static inline u16 u8array_to_u16 (const u8* item, int offset)
{
	return ( item[offset] | (item[offset + 1] << 8));
}
static inline u32 u8array_to_u32 (const u8* item, int offset) 
{
	return ( item[offset] | (item[offset + 1] << 8) | (item[offset + 2] << 16) | (item[offset + 3] << 24) );
}
static inline void u16_to_u8array (u8* item, int offset, u16 value)
{
	item[offset] = (u8) value; item[offset + 1] = (u8)(value >> 8); 
}
static inline void u32_to_u8array (u8* item, int offset, u32 value)
{
	item[offset] = (u8) value;
	item[offset + 1] = (u8)(value >> 8);
	item[offset + 2] = (u8)(value >> 16);
	item[offset + 3] = (u8)(value >> 24);
} 


class padding
{
public:
	padding(u32 Start, u32 NumSectors)
	{
		start = Start;
		end = Start + NumSectors;
		data = new u8[NumSectors*512];
		next = 0;
	}
	~padding() { delete [] data;}

	u32 start;
	u32 end;
	u8 *data;
 	padding *next;
};

static u32 __ramdisk_sectorsOfPadding;
static padding *__ramdisk_data_start;
static padding *__ramdisk_data_end;

static bool __ramdisk_IsInserted(void)
{
	return __ramdisk_data_start!=NULL;
}
//forward decleration
static bool __ramdisk_WriteSectors(size_t Sector, size_t numSectors, u8 *Buffer);

bool initRAMDisc(u32 Size, u32 Padding)
{
	if(__ramdisk_data_start) return true; // is init

	if(Size > 16*1024*1024)	Size = 16*1024*1024; // maximum 16 MB
	if(Size < 16*1024)		Size = 16*1024; // minimum 16 MB
	if(Padding > Size)		Padding = Size; // Padding maximum =Disksize
	if(Padding < 4*1024)	Padding = 4*1024; // Padding minimum 4kB
	
	__ramdisk_sectorsOfPadding = Padding/512;

	__ramdisk_data_start = __ramdisk_data_end = new padding(0, __ramdisk_sectorsOfPadding);
	if(!__ramdisk_data_start) return false;

	// FAT12 Formatieren
	u8 sector[512] = {0, };
	sector[0x00d]	= 2; 	/*BPB_sectorsPerCluster*/
	sector[0x00e]	= 1; 	/*BPB_reservedSectors*/
	sector[0x010]	= 1; 	/*BPB_numFATs*/
	u16_to_u8array (sector, 0x011, 48);	/*BPB_rootEntries*/
	int num_sectors = Size/512;
	u16_to_u8array (sector, 0x013, num_sectors);	/*BPB_numSectorsSmall*/
	int num_clusters = (num_sectors-1-3) /2;
	int sectors_per_fat = (num_clusters * 3 + 1023) /1024;
	u16_to_u8array (sector, 0x016, sectors_per_fat);	/*BPB_sectorsPerFAT*/
	//u32_to_u8array (sector, 0x020, Size/512);	/*BPB_numSectors*/
	sector[0x036] 	= 'F';
	sector[0x037] 	= 'A';
	sector[0x038] 	= 'T';
	sector[0x1fe] 	= 0x55;
	sector[0x1ff] 	= 0xaa;
	if(!__ramdisk_WriteSectors(0, 1, sector))
		goto error;
	memset(sector, 0, 512);
	// clear FAT & rootDir
	for(int i=1; i<= sectors_per_fat+3/*sectors_per_rootDir*/; i++)
		if(!__ramdisk_WriteSectors(1, 1, sector))
			goto error;
	return true;
error:
	delete __ramdisk_data_start;
	__ramdisk_data_start = 0;
	return false;

}

void exitRAMDisc()
{
	while(__ramdisk_data_start)
	{
		padding *tmp = __ramdisk_data_start;
		__ramdisk_data_start = __ramdisk_data_start->next;
		delete tmp;	
	}
}
static u8 *__ramdisk_findSector(size_t Sector, size_t *Sectors)
{
	if(__ramdisk_data_start==NULL) return NULL;
	for(padding *tmp = __ramdisk_data_start; tmp; tmp=tmp->next)
	{
		if(tmp->start <= Sector && tmp->end >= Sector)
		{
			if(Sectors) *Sectors = 1+tmp->end-Sector;
			return &(tmp->data[(Sector-tmp->start)*512]);
		}
	}
	// not found -> add padding
	__ramdisk_data_end->next = new padding((Sector/__ramdisk_sectorsOfPadding)*__ramdisk_sectorsOfPadding, __ramdisk_sectorsOfPadding);
	if(__ramdisk_data_end->next)
	{
		__ramdisk_data_end = __ramdisk_data_end->next;
		return &( __ramdisk_data_end->data[(Sector-__ramdisk_data_end->start)*512]);
	}
	return 0;
}

static bool __ImplizitInit = false;
static bool __ramdisk_Startup(void)
{
	if(!__ramdisk_IsInserted())
	{
		// Std=8MB/64kB Padding
		return (__ImplizitInit = initRAMDisc(8*1024*1024, 64 * 1024));
	}
	return true;
}

static bool __ramdisk_ReadSectors(size_t Sector, size_t numSectors, u8 *Buffer)
{
	size_t num_sectors;
	while(numSectors)
	{
		if(u8 *buff = __ramdisk_findSector(Sector,&num_sectors))
		{
			if(num_sectors > numSectors) num_sectors = numSectors;
			memcpy(Buffer, buff, num_sectors * 512);
			numSectors -= num_sectors;
			Buffer+= num_sectors;
		}
		else
			return false;
	}
	return true;
}
static bool __ramdisk_WriteSectors(size_t Sector, size_t numSectors, u8 *Buffer)
{
	size_t num_sectors;
	while(numSectors)
	{
		if(u8 *buff = __ramdisk_findSector(Sector,&num_sectors))
		{
			if(num_sectors > numSectors) num_sectors = numSectors;
			memcpy(buff, Buffer, num_sectors * 512);
			numSectors -= num_sectors;
			Buffer+= num_sectors;
		}
		else
			return false;
	}
	return true;
}
static bool __ramdisk_ClearStatus(void)
{
	return true;
}
static bool __ramdisk_Shutdown(void)
{
	if(__ImplizitInit)
	{
		__ImplizitInit = false;
		exitRAMDisc();
	}
	return true;
}

const DISC_INTERFACE __io_ramdisk = {
	DEVICE_TYPE_RAM_DISK,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | 0x1000,
	(FN_MEDIUM_STARTUP)&__ramdisk_Startup,
	(FN_MEDIUM_ISINSERTED)&__ramdisk_IsInserted,
	(FN_MEDIUM_READSECTORS)&__ramdisk_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&__ramdisk_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&__ramdisk_ClearStatus, 
	(FN_MEDIUM_SHUTDOWN)&__ramdisk_Shutdown
}; 
