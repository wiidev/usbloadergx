#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <gccore.h>
#include <ogcsys.h>

#include "utils.h"
#include "../settings/cfg.h"
#include "fatmounter.h"
#include "id.h"
#include "isfs.h"


#define MAX_TITLES 256


s32 Title_GetList(u64 **outbuf, u32 *outlen) {
    u64 *titles = NULL;

    u32 len, nb_titles;
    s32 ret;

    /* Get number of titles */
    ret = ES_GetNumTitles(&nb_titles);
    if (ret < 0)
        return ret;

    /* Calculate buffer lenght */
    len = round_up(sizeof(u64) * nb_titles, 32);

    /* Allocate memory */
    titles = memalign(32, len);
    if (!titles)
        return -1;

    /* Get titles */
    ret = ES_GetTitles(titles, nb_titles);
    if (ret < 0)
        goto err;

    /* Set values */
    *outbuf = titles;
    *outlen = nb_titles;

    return 0;

err:
    /* Free memory */
    if (titles)
        free(titles);

    return ret;
}

s32 Title_GetTicketViews(u64 tid, tikview **outbuf, u32 *outlen) {
    tikview *views = NULL;

    u32 nb_views;
    s32 ret;

    /* Get number of ticket views */
    ret = ES_GetNumTicketViews(tid, &nb_views);
    if (ret < 0)
        return ret;

    /* Allocate memory */
    views = (tikview *)memalign(32, sizeof(tikview) * nb_views);
    if (!views)
        return -1;

    /* Get ticket views */
    ret = ES_GetTicketViews(tid, views, nb_views);
    if (ret < 0)
        goto err;

    /* Set values */
    *outbuf = views;
    *outlen = nb_views;

    return 0;

err:
    /* Free memory */
    if (views)
        free(views);

    return ret;
}

s32 Title_GetTMD(u64 tid, signed_blob **outbuf, u32 *outlen) {
    void *p_tmd = NULL;

    u32 len;
    s32 ret;

    /* Get TMD size */
    ret = ES_GetStoredTMDSize(tid, &len);
    if (ret < 0)
        return ret;

    /* Allocate memory */
    p_tmd = memalign(32, round_up(len, 32));
    if (!p_tmd)
        return -1;

    /* Read TMD */
    ret = ES_GetStoredTMD(tid, p_tmd, len);
    if (ret < 0)
        goto err;

    /* Set values */
    *outbuf = p_tmd;
    *outlen = len;

    return 0;

err:
    /* Free memory */
    if (p_tmd)
        free(p_tmd);

    return ret;
}

s32 Title_GetVersion(u64 tid, u16 *outbuf) {
    signed_blob *p_tmd = NULL;
    tmd      *tmd_data = NULL;

    u32 len;
    s32 ret;

    /* Get title TMD */
    ret = Title_GetTMD(tid, &p_tmd, &len);
    if (ret < 0)
        return ret;

    /* Retrieve TMD info */
    tmd_data = (tmd *)SIGNATURE_PAYLOAD(p_tmd);

    /* Set values */
    *outbuf = tmd_data->title_version;

    /* Free memory */
    free(p_tmd);

    return 0;
}

s32 Title_GetSysVersion(u64 tid, u64 *outbuf) {
    signed_blob *p_tmd = NULL;
    tmd      *tmd_data = NULL;

    u32 len;
    s32 ret;

    /* Get title TMD */
    ret = Title_GetTMD(tid, &p_tmd, &len);
    if (ret < 0)
        return ret;

    /* Retrieve TMD info */
    tmd_data = (tmd *)SIGNATURE_PAYLOAD(p_tmd);

    /* Set values */
    *outbuf = tmd_data->sys_version;

    /* Free memory */
    free(p_tmd);

    return 0;
}



s32 Title_GetSize(u64 tid, u32 *outbuf) {
    signed_blob *p_tmd = NULL;
    tmd      *tmd_data = NULL;

    u32 cnt, len, size = 0;
    s32 ret;

    /* Get title TMD */
    ret = Title_GetTMD(tid, &p_tmd, &len);
    if (ret < 0)
        return ret;

    /* Retrieve TMD info */
    tmd_data = (tmd *)SIGNATURE_PAYLOAD(p_tmd);

    /* Calculate title size */
    for (cnt = 0; cnt < tmd_data->num_contents; cnt++) {
        tmd_content *content = &tmd_data->contents[cnt];

        /* Add content size */
        size += content->size;
    }

    /* Set values */
    *outbuf = size;

    /* Free memory */
    free(p_tmd);

    return 0;
}

s32 Title_GetIOSVersions(u8 **outbuf, u32 *outlen) {
    u8  *buffer = NULL;
    u64 *list   = NULL;

    u32 count, cnt, idx;
    s32 ret;

    /* Get title list */
    ret = Title_GetList(&list, &count);
    if (ret < 0)
        return ret;

    /* Count IOS */
    for (cnt = idx = 0; idx < count; idx++) {
        u32 tidh = (list[idx] >> 32);
        u32 tidl = (list[idx] &  0xFFFFFFFF);

        /* Title is IOS */
        if ((tidh == 0x1) && (tidl >= 3) && (tidl <= 255))
            cnt++;
    }

    /* Allocate memory */
    buffer = (u8 *)memalign(32, cnt);
    if (!buffer) {
        ret = -1;
        goto out;
    }

    /* Copy IOS */
    for (cnt = idx = 0; idx < count; idx++) {
        u32 tidh = (list[idx] >> 32);
        u32 tidl = (list[idx] &  0xFFFFFFFF);

        /* Title is IOS */
        if ((tidh == 0x1) && (tidl >= 3) && (tidl <= 255))
            buffer[cnt++] = (u8)(tidl & 0xFF);
    }

    /* Set values */
    *outbuf = buffer;
    *outlen = cnt;

    goto out;

out:
    /* Free memory */
    if (list)
        free(list);

    return ret;
}

s32 Uninstall_RemoveTicket(u64 tid) {
    static tikview viewdata[0x10] ATTRIBUTE_ALIGN(32);

    u32 cnt, views;
    s32 ret;

    /* Get number of ticket views */
    ret = ES_GetNumTicketViews(tid, &views);
    if (ret < 0) {

        return ret;
    }

    if (!views) {
        //printf(" No tickets found!\n");
        return 1;
    } else if (views > 16) {
        //printf(" Too many ticket views! (views = %d)\n", views);
        return -1;
    }

    /* Get ticket views */
    ret = ES_GetTicketViews(tid, viewdata, views);
    if (ret < 0) {
        //printf(" \n\tError! ES_GetTicketViews (ret = %d)\n", ret);
        return ret;
    }

    /* Remove tickets */
    for (cnt = 0; cnt < views; cnt++) {
        ret = ES_DeleteTicket(&viewdata[cnt]);
        if (ret < 0) {
            //printf(" Error! (view = %d, ret = %d)\n", cnt, ret);
            return ret;
        }
    }
    //printf(" OK!\n");

    return ret;
}

s32 Uninstall_DeleteTitle(u32 title_u, u32 title_l) {
    s32 ret;
    char filepath[256];
    sprintf(filepath, "/title/%08x/%08x",  title_u, title_l);

    /* Remove title */
    ret = ISFS_Delete(filepath);
    return ret;
}

s32 Uninstall_DeleteTicket(u32 title_u, u32 title_l) {
    s32 ret;

    char filepath[256];
    sprintf(filepath, "/ticket/%08x/%08x.tik", title_u, title_l);

    /* Delete ticket */
    ret = ISFS_Delete(filepath);

    return ret;
}


//////savegame shit, from waninkoko.  modified for use in this project
/* Savegame structure */
struct savegame {
	/* Title name */
	char name[65];

	/* Title ID */
	u64 tid;
};

s32 Savegame_CheckTitle(const char *path)
{
	FILE *fp = NULL;

	char filepath[128];

	/* Generate filepath */
	sprintf(filepath, "%s/banner.bin", path);

	/* Try to open banner */
	fp = fopen(filepath, "rb");
	if (!fp)
		return -1;

	/* Close file */
	fclose(fp);

	return 0;
}

s32 Savegame_GetNandPath(u64 tid, char *outbuf)
{
	s32 ret;
	char buffer[1024] ATTRIBUTE_ALIGN(32);

	/* Get data directory */
	ret = ES_GetDataDir(tid, buffer);
	if (ret < 0)
		return ret;

	/* Generate NAND directory */
	sprintf(outbuf, "isfs:%s", buffer);

	return 0;
}

s32 __Menu_GetNandSaves(struct savegame **outbuf, u32 *outlen)
{
	struct savegame *buffer = NULL;

	u64 *titleList = NULL;
	u32  titleCnt;

	u32 cnt, idx;
	s32 ret;

	/* Get title list */
	ret = Title_GetList(&titleList, &titleCnt);
	if (ret < 0)
		return ret;

	/* Allocate memory */
	buffer = malloc(sizeof(struct savegame) * titleCnt);
	if (!buffer) {
		ret = -1;
		goto out;
	}

	/* Copy titles */
	for (cnt = idx = 0; idx < titleCnt; idx++) {
		u64  tid = titleList[idx];
		char savepath[128];

		/* Generate dirpath */
		Savegame_GetNandPath(tid, savepath);

		/* Check for title savegame */
		ret = Savegame_CheckTitle(savepath);
		if (!ret) {
			struct savegame *save = &buffer[cnt++];

			/* Set title ID */
			save->tid = tid;
		}
	}

	/* Set values */
	*outbuf = buffer;
	*outlen = cnt;

	/* Success */
	ret = 0;

out:
	/* Free memory */
	if (titleList)
		free(titleList);

	return ret;
}

s32 __Menu_EntryCmp(const void *p1, const void *p2)
{
	struct savegame *s1 = (struct savegame *)p1;
	struct savegame *s2 = (struct savegame *)p2;

	/* Compare entries */
	return strcmp(s1->name, s2->name);
}

s32 __Menu_RetrieveList(struct savegame **outbuf, u32 *outlen)
{
	s32 ret;
	ret = __Menu_GetNandSaves(outbuf, outlen);
	if (ret >= 0)
		qsort(*outbuf, *outlen, sizeof(struct savegame), __Menu_EntryCmp);

	return ret;
}

//carefull when using this function
//it will force remove stuff even if something fails
s32 Uninstall_FromTitle(const u64 tid) {
    s32 contents_ret, tik_ret, title_ret, ret;
    u32 id = tid & 0xFFFFFFFF, kind = tid >> 32;
    contents_ret = tik_ret = title_ret = ret = 0;

    if (kind == 1) {
        // Delete title and ticket at FS level.
        tik_ret		= Uninstall_DeleteTicket(kind, id);
        title_ret	= Uninstall_DeleteTitle(kind, id);
        contents_ret = title_ret;
    } else {
        // Remove title (contents and ticket)
        tik_ret		= Uninstall_RemoveTicket(tid);
        contents_ret	= ES_DeleteTitleContent(tid);
        title_ret	= ES_DeleteTitle(tid);


        // Attempt forced uninstall if something fails
        if (tik_ret < 0 || contents_ret < 0 || title_ret < 0) {
            tik_ret		= Uninstall_DeleteTicket(kind, id);
            title_ret	= Uninstall_DeleteTitle(kind, id);
            contents_ret = title_ret;

        }
    }
    if (tik_ret < 0 && contents_ret < 0 && title_ret < 0)
        ret = -1;
    else if (tik_ret < 0 || contents_ret < 0 || title_ret < 0)
        ret =  1;
    else
        ret =  0;

    return ret;
}


/*-------------------------------------------------------------
 taken from anytitledeleter
 name.c -- functions for determining the name of a title

 Copyright (C) 2009 MrClick

-------------------------------------------------------------*/

s32 __convertWiiString(char *str, u8 *data, u32 cnt) {
    u32 i = 0;
    for (; i < cnt; data += 2) {
        u16 *chr = (u16*)data;
        if (*chr == 0)
            break;
        // ignores all but ASCII characters
        else if (*chr >= 0x20 && *chr <= 0x7E)
            str[i] = *chr;
        else
            str[i] = '.';
        i++;
    }
    str[i] = 0;

    return 0;
}


s32 getNameBN(char* name, u64 id) {
    // Terminate the name string just in case the function exits prematurely
    name[0] = 0;

    // Create a string containing the absolute filename
    char file[256] __attribute__ ((aligned (32)));
    sprintf(file, "/title/%08x/%08x/data/banner.bin", (u32)(id >> 32), (u32)id);

    // Bring the Wii into the title's userspace
    if (ES_SetUID(id) < 0) {
        // Should that fail repeat after setting permissions to system menu mode
        Identify_SysMenu();
        if (ES_SetUID(id) < 0)
            return -1;
    }

    // Try to open file
    s32 fh = ISFS_Open(file, ISFS_OPEN_READ);

    // If a title does not have a banner.bin bail out
    if (fh == -106)
        return -2;

    // If it fails try to open again after identifying as SU
    if (fh == -102) {
        Identify_SU();
        fh = ISFS_Open(file, ISFS_OPEN_READ);
    }
    // If the file won't open
    else if (fh < 0)
        return fh;

    // Seek to 0x20 where the name is stored
    ISFS_Seek(fh, 0x20, 0);

    // Read a chunk of 256 bytes from the banner.bin
    u8 *data = memalign(32, 0x100);
    if (ISFS_Read(fh, data, 0x100) < 0) {
        ISFS_Close(fh);
        free(data);
        return -3;
    }


    // Prepare the strings that will contain the name of the title
    char name1[0x41] __attribute__ ((aligned (32)));
    char name2[0x41] __attribute__ ((aligned (32)));
    name1[0x40] = 0;
    name2[0x40] = 0;

    __convertWiiString(name1, data + 0x00, 0x40);
    __convertWiiString(name2, data + 0x40, 0x40);
    free(data);

    // Assemble name
    sprintf(name, "%s", name1);
    if (strlen(name2) > 1)
        sprintf(name, "%s (%s)", name, name2);

    // Close the banner.bin
    ISFS_Close(fh);

    // Job well done
    return 1;
}


s32 getName00(char* name, u64 id, int lang) {
    /*
    languages
    0jap
    2eng
    4german
    6french
    8spanish
    10italian
    12dutch
    */
    // Create a string containing the absolute filename
    char file[256] __attribute__ ((aligned (32)));
    sprintf(file, "/title/%08x/%08x/content/00000000.app", (u32)(id >> 32), (u32)id);
    Identify_SU();
    s32 fh = ISFS_Open(file, ISFS_OPEN_READ);



    // If the title does not have 00000000.app bail out
    if (fh == -106)
        return fh;

    // In case there is some problem with the permission
    if (fh == -102) {
        // Identify as super user
        Identify_SU();
        fh = ISFS_Open(file, ISFS_OPEN_READ);
    } else if (fh < 0)
        return fh;

    // Jump to start of the name entries
    ISFS_Seek(fh, 0x9C, 0);

    // Read a chunk of 0x22 * 0x2B bytes from 00000000.app
    u8 *data = memalign(32, 2048);
    s32 r = ISFS_Read(fh, data, 0x22 * 0x2B);
    //printf("%s %d\n", file, r);wait_anyKey();
    if (r < 0) {
        ISFS_Close(fh);
        free(data);
        return -4;
    }

    // Take the entries apart
    char str[0x22][0x2B];
    u8 i = 0;
    // Convert the entries to ASCII strings
    for (; i < 0x22; i++)
        __convertWiiString(str[i], data + (i * 0x2A), 0x2A);

    // Clean up
    ISFS_Close(fh);
    free(data);

    // Assemble name
    if (strlen(str[lang]) > 1) {
        sprintf(name, "%s", str[lang]);
        if (strlen(str[lang+1]) > 1)
            sprintf(name, "%s (%s)", name, str[lang+1]);
    } else {
        sprintf(name, "%s", str[2]);
        if (strlen(str[3]) > 1)
            sprintf(name, "%s (%s)", name, str[3]);
    }
    // Job well done
    return 2;
}


s32 printContent(u64 tid) {
    char dir[256] __attribute__ ((aligned (32)));
    sprintf(dir, "/title/%08x/%08x/content", (u32)(tid >> 32), (u32)tid);

    u32 num = 64;

    static char list[8000] __attribute__((aligned(32)));

    ISFS_ReadDir(dir, list, &num);

    char *ptr = list;
    u8 br = 0;
    for (; strlen(ptr) > 0; ptr += strlen(ptr) + 1) {
        printf("     %-12.12s", ptr);
        br++;
        if (br == 4) {
            br = 0;
            printf("\n");
        }
    }
    if (br != 0)
        printf("\n");

    return num;
}


char *titleText(u32 kind, u32 title) {
    static char text[10];

    if (kind == 1) {
        // If we're dealing with System Titles, use custom names
        switch (title) {
        case 1:
            strcpy(text, "BOOT2");
            break;
        case 2:
            strcpy(text, "SYSMENU");
            break;
        case 0x100:
            strcpy(text, "BC");
            break;
        case 0x101:
            strcpy(text, "MIOS");
            break;
        default:
            sprintf(text, "IOS%u", title);
            break;
        }
    } else {
        // Otherwise, just convert the title to ASCII
        int i =32, j = 0;
        do {
            u8 temp;
            i -= 8;
            temp = (title >> i) & 0x000000FF;
            if (temp < 32 || temp > 126)
                text[j] = '.';
            else
                text[j] = temp;
            j++;
        } while (i > 0);
        text[4] = 0;
    }
    return text;
}

//giantpune's magic function to check for game saves
//give a ID4 of a game and returns 1 if the game has save data, 0 if not, or <0 for errors
int CheckForSave(const char *gameID)
{

	if (ISFS_Initialize()<0)
		return -1;
	
	if (!ISFS_Mount())
		return -2;
		
	struct savegame *saveList = NULL;
	u32 saveCnt;
	u32 cnt;
	
	
	if (__Menu_RetrieveList(&saveList, &saveCnt)<0)
		return -3;

	for (cnt=0;cnt<saveCnt;cnt++)
	{	
		struct savegame *save = &saveList[cnt];
		if (strcmp(gameID,titleText((u32)(save->tid >> 32),(u32)(save->tid & 0xFFFFFFFF)))==0) {
			free(saveList);
			return 1;
		}
	}
	free(saveList);
	return 0;

}


/*-------------------------------------------------------------
 from any title deleter
titles.c -- functions for grabbing all titles of a certain type

Copyright (C) 2008 tona
-------------------------------------------------------------*/

u32 __titles_init = 0;
u32 __num_titles;
static u64 __title_list[MAX_TITLES] ATTRIBUTE_ALIGN(32);

s32 __getTitles() {
    s32 ret;
    ret = ES_GetNumTitles(&__num_titles);
    if (ret <0)
        return ret;
    if (__num_titles > MAX_TITLES)
        return -1;
    ret = ES_GetTitles(__title_list, __num_titles);
    if (ret <0)
        return ret;
    __titles_init = 1;
    return 0;
}

s32 getTitles_TypeCount(u32 type, u32 *count) {
    s32 ret = 0;
    u32 type_count;
    if (!__titles_init)
        ret = __getTitles();
    if (ret <0)
        return ret;
    int i;
    type_count = 0;
    for (i=0; i < __num_titles; i++) {
        u32 upper, lower;
        upper = __title_list[i] >> 32;
        lower = __title_list[i] & 0xFFFFFFFF;
        if ((upper == type)&&
                ((lower !=0x48414741)&&//this filters out haga,haaa, hafa.  dupe factory channels that don't load
                 (lower !=0x48414141)&&//since we dont care about apps that dont load for what we are doing
                 (lower !=0x48414641)))
            type_count++;
    }
    *count = type_count;
    return ret;
}

s32 getTitles_Type(u32 type, u32 *titles, u32 count) {
    s32 ret = 0;
    u32 type_count;
    if (!__titles_init)
        ret = __getTitles();
    if (ret <0)
        return ret;
    int i;
    type_count = 0;
    for (i=0; type_count < count && i < __num_titles; i++) {
        u32 upper, lower;
        upper = __title_list[i] >> 32;
        lower = __title_list[i] & 0xFFFFFFFF;
        if ((upper == type)&&
                ((lower !=0x48414741)&&
                 (lower !=0x48414141)&&
                 (lower !=0x48414641))) {
            titles[type_count]=lower;
            type_count++;
        }
    }
    if (type_count < count)
        return -2;
    __titles_init = 0;
    return 0;
}


//this function expects initialize be called before it is called
// if not, it will fail miserably and catch the wii on fire and kick you in the nuts
#define TITLE_ID(x,y)		(((u64)(x) << 32) | (y))
s32 WII_BootHBC()
{
	u32 tmdsize;
	u64 tid = 0;
	u64 *list;
	u32 titlecount;
	s32 ret;
	u32 i;

	ret = ES_GetNumTitles(&titlecount);
	if(ret < 0)
		return WII_EINTERNAL;

	list = memalign(32, titlecount * sizeof(u64) + 32);

	ret = ES_GetTitles(list, titlecount);
	if(ret < 0) {
		free(list);
		return WII_EINTERNAL;
	}
	
	for(i=0; i<titlecount; i++) {
		if (list[i]==TITLE_ID(0x00010001,0x4A4F4449) 
			|| list[i]==TITLE_ID(0x00010001,0x48415858))
		{
			tid = list[i];
			break;
		}
	}
	free(list);

	if(!tid)
		return WII_EINSTALL;

	if(ES_GetStoredTMDSize(tid, &tmdsize) < 0)
		return WII_EINSTALL;

	return WII_LaunchTitle(tid);
}



