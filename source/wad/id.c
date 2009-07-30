/*-------------------------------------------------------------
 
id.c -- ES Identification code
 
Copyright (C) 2008 tona
Unless other credit specified
 
This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.
 
Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:
 
1.The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.
 
2.Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.
 
3.This notice may not be removed or altered from any source
distribution.
 
-------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <gccore.h>

#include "id.h"
#include "patchmii_core.h"

#include "certs_dat.h"


// Turn upper and lower into a full title ID
#define TITLE_ID(x,y)		(((u64)(x) << 32) | (y))
// Get upper or lower half of a title ID
#define TITLE_UPPER(x)		((u32)((x) >> 32))
// Turn upper and lower into a full title ID
#define TITLE_LOWER(x)		((u32)(x))


/* Debug functions adapted from libogc's es.c */
//#define DEBUG_ES
//#define DEBUG_IDENT
#define ISALIGNED(x) ((((u32)x)&0x1F)==0)

static u8 su_tmd[0x208] ATTRIBUTE_ALIGN(32);
static u8 su_tik[STD_SIGNED_TIK_SIZE] ATTRIBUTE_ALIGN(32);
int su_id_filled = 0;


/* Reads a file from ISFS to an array in memory */
s32 ISFS_ReadFileToArray (const char *filepath, u8 *filearray, u32 max_size, u32 *file_size) {
	s32 ret, fd;
	static fstats filestats ATTRIBUTE_ALIGN(32);
	
	*file_size = 0;
	ret = ISFS_Open(filepath, ISFS_OPEN_READ);
	if (ret <= 0)
	{
		//printf("Error! ISFS_Open (ret = %d)\n", ret);
		return -1;
	}
	
	fd = ret;
	
	ret = ISFS_GetFileStats(fd, &filestats);
	if (ret < 0)
	{
		//printf("Error! ISFS_GetFileStats (ret = %d)\n", ret);
		return -1;
	}
	
	*file_size = filestats.file_length;
	
	if (*file_size > max_size)
	{
		//printf("File is too large! Size: %u Max: %u", *file_size, max_size);
		return -1;
	}
	
	ret = ISFS_Read(fd, filearray, *file_size);
	*file_size = ret;
	if (ret < 0)
	{
		//printf("Error! ISFS_Read (ret = %d)\n", ret);
		return -1;
	} 
	else if (ret != filestats.file_length)
	{
		//printf("Error! ISFS_Read Only read: %d\n", ret);
		return -1;
	}
	
	ret = ISFS_Close(fd);
	if (ret < 0)
	{
		//printf("Error! ISFS_Close (ret = %d)\n", ret);
		return -1;
	}
	return 0;
}

void Make_SUID(void){
	signed_blob *s_tmd, *s_tik;
	tmd *p_tmd;
	tik *p_tik;
	
	memset(su_tmd, 0, sizeof su_tmd);
	memset(su_tik, 0, sizeof su_tik);
	s_tmd = (signed_blob*)&su_tmd[0];
	s_tik = (signed_blob*)&su_tik[0];
	*s_tmd = *s_tik = 0x10001;
	p_tmd = (tmd*)SIGNATURE_PAYLOAD(s_tmd);
	p_tik = (tik*)SIGNATURE_PAYLOAD(s_tik);
	
	
	strcpy(p_tmd->issuer, "Root-CA00000001-CP00000004");
	p_tmd->title_id = TITLE_ID(1,2);
	
	p_tmd->num_contents = 1;
	
	forge_tmd(s_tmd);
	
	strcpy(p_tik->issuer, "Root-CA00000001-XS00000003");
	p_tik->ticketid = 0x000038A45236EE5FLL;
	p_tik->titleid = TITLE_ID(1,2);
	
	memset(p_tik->cidx_mask, 0xFF, 0x20);
	forge_tik(s_tik);
	
	su_id_filled = 1;
	
}

s32 Identify(const u8 *certs, u32 certs_size, const u8 *idtmd, u32 idtmd_size, const u8 *idticket, u32 idticket_size) {
	s32 ret;
	u32 keyid = 0;
	ret = ES_Identify((signed_blob*)certs, certs_size, (signed_blob*)idtmd, idtmd_size, (signed_blob*)idticket, idticket_size, &keyid);
	/*if (ret < 0){
		switch(ret){
			case ES_EINVAL:
				printf("Error! ES_Identify (ret = %d;) Data invalid!\n", ret);
				break;
			case ES_EALIGN:
				printf("Error! ES_Identify (ret = %d;) Data not aligned!\n", ret);
				break;
			case ES_ENOTINIT:
				printf("Error! ES_Identify (ret = %d;) ES not initialized!\n", ret);
				break;
			case ES_ENOMEM:
				printf("Error! ES_Identify (ret = %d;) No memory!\n", ret);
				break;
			default:
				printf("Error! ES_Identify (ret = %d)\n", ret);
				break;
		}
	}
	else
		printf("OK!\n");*/
	return ret;
}


s32 Identify_SU(void) {
	if (!su_id_filled)
		Make_SUID();
	
	//printf("\nIdentifying as SU...");
	//fflush(stdout);
	return Identify(certs_dat, certs_dat_size, su_tmd, sizeof su_tmd, su_tik, sizeof su_tik);
}

s32 Identify_SysMenu(void) {
	s32 ret;
	u32 sysmenu_tmd_size, sysmenu_ticket_size;
	//static u8 certs[0xA00] ATTRIBUTE_ALIGN(32);
	static u8 sysmenu_tmd[MAX_SIGNED_TMD_SIZE] ATTRIBUTE_ALIGN(32);
	static u8 sysmenu_ticket[STD_SIGNED_TIK_SIZE] ATTRIBUTE_ALIGN(32);
	
	/*printf("\nPulling Certs...");
	ret = ISFS_ReadFileToArray ("/sys/certs.sys", certs, 0xA00, &certs_size);
	if (ret < 0) {
		printf("\tReading Certs failed!\n");
		return -1;
	}*/
	
	//printf("\nPulling Sysmenu TMD...");
	ret = ISFS_ReadFileToArray ("/title/00000001/00000002/content/title.tmd", sysmenu_tmd, MAX_SIGNED_TMD_SIZE, &sysmenu_tmd_size);
	if (ret < 0) {
		//printf("\tReading TMD failed!\n");
		return -1;
	}
	
	//printf("\nPulling Sysmenu Ticket...");
	ret = ISFS_ReadFileToArray ("/ticket/00000001/00000002.tik", sysmenu_ticket, STD_SIGNED_TIK_SIZE, &sysmenu_ticket_size);
	if (ret < 0) {
		//printf("\tReading TMD failed!\n");
		return -1;
	}
	
	//printf("\nIdentifying as SysMenu...");
	//fflush(stdout);
	return Identify(certs_dat, certs_dat_size, sysmenu_tmd, sysmenu_tmd_size, sysmenu_ticket, sysmenu_ticket_size);
}
