/*
 *  Copyright (C) 2008 Nuke (wiinuke@gmail.com)
 *
 *  this file is part of GeckoOS for USB Gecko
 *  http://www.usbgecko.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <malloc.h>
#include <sys/unistd.h>
#include <ogc/ipc.h>

#include "fst.h"
#include "dvd_broadway.h"
#include "mload/mload.h"
#include "mload/mload_modules.h"
#include "gecko.h"

#include "patchcode.h"
#include "language/gettext.h"

#include "codehandler.h"
//#include "codehandlerslota.h"
#include "codehandleronly.h"
#include "multidol.h"

#define FSTDIRTYPE 1
#define FSTFILETYPE 0
#define ENTRYSIZE 0xC
//#define FILEDIR   "fat0:/codes"
//#define FILEDIR   "sd:/codes"
#define FILEDIR "/codes"

#define MAX_FILENAME_LEN    128

const char * CheatFilepath = NULL;
static const char * BCAFilepath = NULL;

static u8 *codelistend = NULL;
static void *codelist = NULL;

static u8 *code_buf = NULL;
static int code_size = 0;

static u32 gameconfsize = 0;
static u32 *gameconf = NULL;

static u8 debuggerselect = 0;

extern const u32 viwiihooks[4];
extern const u32 kpadhooks[4];
extern const u32 joypadhooks[4];
extern const u32 gxdrawhooks[4];
extern const u32 gxflushhooks[4];
extern const u32 ossleepthreadhooks[4];
extern const u32 axnextframehooks[4];
extern const u32 wpadbuttonsdownhooks[4];
extern const u32 wpadbuttonsdown2hooks[4];

void SetCheatFilepath(const char * path)
{
    CheatFilepath = path;
}

void SetBCAFilepath(const char * path)
{
    BCAFilepath = path;
}

//static vu32 dvddone = 0;

//---------------------------------------------------------------------------------
void app_loadgameconfig(char *discid)
//---------------------------------------------------------------------------------
{
    if (!CheatFilepath) return;

    gameconfsize = 0;

    if (gameconf == NULL)
    {
        gameconf = (u32*) malloc(65536);
        if (gameconf == NULL)
        {
            //TODO for oggzee
            //print_status("Out of memory");
            return;
        }
    }

    FILE* fp;
    u32 ret;
    u32 filesize;
    s32 gameidmatch, maxgameidmatch = -1, maxgameidmatch2 = -1;
    u32 i, numnonascii, parsebufpos;
    u32 codeaddr, codeval, codeaddr2, codeval2, codeoffset;
    u32 temp, tempoffset = 0;
    char parsebuffer[18];

    //if (config_bytes[2] == 8)
    //  hookset = 1;

    u8 *tempgameconf;
    u32 tempgameconfsize = 0;

    //memcpy(tempgameconf, defaultgameconfig, defaultgameconfig_size);
    //tempgameconf[defaultgameconfig_size] = '\n';
    //tempgameconfsize = defaultgameconfig_size + 1;

    char filepath[200];
    snprintf(filepath, sizeof(filepath), "%s/gameconfig.txt", CheatFilepath);

    fp = fopen(filepath, "rb");

    if (!fp)
    {
        snprintf(filepath, sizeof(filepath), "sd:/gameconfig.txt");
        fp = fopen(filepath, "rb");
        if (!fp)
        {
            snprintf(filepath, sizeof(filepath), "usb:/gameconfig.txt");
            fp = fopen(filepath, "rb");
        }
    }

    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        filesize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        tempgameconf = (u8*) malloc(filesize);
        if (tempgameconf == NULL)
        {
            //TODO for oggzee
            //print_status("Out of memory");
            //wait(4);
            return;
        }

        ret = fread((void*) tempgameconf, 1, filesize, fp);
        fclose(fp);
        if (ret != filesize)
        {
            //TODO for oggzee
            //print_status("Error reading gameconfig.txt");
            //wait(4);
            return;
        }
        tempgameconfsize = filesize;
    }
    else
    {
        return;
    }

    // Remove non-ASCII characters
    numnonascii = 0;
    for (i = 0; i < tempgameconfsize; i++)
    {
        if (tempgameconf[i] < 9 || tempgameconf[i] > 126)
            numnonascii++;
        else tempgameconf[i - numnonascii] = tempgameconf[i];
    }
    tempgameconfsize -= numnonascii;

    *(tempgameconf + tempgameconfsize) = 0;
    //gameconf = (tempgameconf + tempgameconfsize) + (4 - (((u32) (tempgameconf + tempgameconfsize)) % 4));

    for (maxgameidmatch = 0; maxgameidmatch <= 6; maxgameidmatch++)
    {
        i = 0;
        while (i < tempgameconfsize)
        {
            maxgameidmatch2 = -1;
            while (maxgameidmatch != maxgameidmatch2)
            {
                while (i != tempgameconfsize && tempgameconf[i] != ':')
                    i++;
                if (i == tempgameconfsize) break;
                while ((tempgameconf[i] != 10 && tempgameconf[i] != 13) && (i != 0))
                    i--;
                if (i != 0) i++;
                parsebufpos = 0;
                gameidmatch = 0;
                while (tempgameconf[i] != ':')
                {
                    if (tempgameconf[i] == '?')
                    {
                        parsebuffer[parsebufpos] = discid[parsebufpos];
                        parsebufpos++;
                        gameidmatch--;
                        i++;
                    }
                    else if (tempgameconf[i] != 0 && tempgameconf[i] != ' ')
                        parsebuffer[parsebufpos++] = tempgameconf[i++];
                    else if (tempgameconf[i] == ' ')
                        break;
                    else i++;
                    if (parsebufpos == 8) break;
                }
                parsebuffer[parsebufpos] = 0;
                if (strncasecmp("DEFAULT", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 7)
                {
                    gameidmatch = 0;
                    goto idmatch;
                }
                if (strncmp(discid, parsebuffer, strlen(parsebuffer)) == 0)
                {
                    gameidmatch += strlen(parsebuffer);
                    idmatch: if (gameidmatch > maxgameidmatch2)
                    {
                        maxgameidmatch2 = gameidmatch;
                    }
                }
                while ((i != tempgameconfsize) && (tempgameconf[i] != 10 && tempgameconf[i] != 13))
                    i++;
            }
            while (i != tempgameconfsize && tempgameconf[i] != ':')
            {
                parsebufpos = 0;
                while ((i != tempgameconfsize) && (tempgameconf[i] != 10 && tempgameconf[i] != 13))
                {
                    if (tempgameconf[i] != 0 && tempgameconf[i] != ' ' && tempgameconf[i] != '(' && tempgameconf[i]
                            != ':')
                        parsebuffer[parsebufpos++] = tempgameconf[i++];
                    else if (tempgameconf[i] == ' ' || tempgameconf[i] == '(' || tempgameconf[i] == ':')
                        break;
                    else i++;
                    if (parsebufpos == 17) break;
                }
                parsebuffer[parsebufpos] = 0;
                //if (!autobootcheck)
                {
                    //if (strncasecmp("addtocodelist(", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 14)
                    //{
                    //  ret = sscanf(tempgameconf + i, "%x %x", &codeaddr, &codeval);
                    //  if (ret == 2)
                    //      addtocodelist(codeaddr, codeval);
                    //}
                    if (strncasecmp("codeliststart", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer)
                            == 13)
                    {
                        sscanf((char *) (tempgameconf + i), " = %x", (unsigned int *) &codelist);
                    }
                    if (strncasecmp("codelistend", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 11)
                    {
                        sscanf((char *) (tempgameconf + i), " = %x", (unsigned int *) &codelistend);
                    }
                    /*
                     if (strncasecmp("hooktype", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 8)
                     {
                     if (hookset == 1)
                     {
                     ret = sscanf(tempgameconf + i, " = %u", &temp);
                     if (ret == 1)
                     if (temp >= 0 && temp <= 7)
                     config_bytes[2] = temp;
                     }
                     }
                     */
                    if (strncasecmp("poke", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 4)
                    {
                        ret = sscanf((char *) tempgameconf + i, "( %x , %x", &codeaddr, &codeval);
                        if (ret == 2)
                        {
                            *(gameconf + (gameconfsize / 4)) = 0;
                            gameconfsize += 4;
                            *(gameconf + (gameconfsize / 4)) = 0;
                            gameconfsize += 8;
                            *(gameconf + (gameconfsize / 4)) = codeaddr;
                            gameconfsize += 4;
                            *(gameconf + (gameconfsize / 4)) = codeval;
                            gameconfsize += 4;
                            DCFlushRange((void *) (gameconf + (gameconfsize / 4) - 5), 20);
                        }
                    }
                    if (strncasecmp("pokeifequal", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 11)
                    {
                        ret = sscanf((char *) (tempgameconf + i), "( %x , %x , %x , %x", &codeaddr, &codeval,
                                &codeaddr2, &codeval2);
                        if (ret == 4)
                        {
                            *(gameconf + (gameconfsize / 4)) = 0;
                            gameconfsize += 4;
                            *(gameconf + (gameconfsize / 4)) = codeaddr;
                            gameconfsize += 4;
                            *(gameconf + (gameconfsize / 4)) = codeval;
                            gameconfsize += 4;
                            *(gameconf + (gameconfsize / 4)) = codeaddr2;
                            gameconfsize += 4;
                            *(gameconf + (gameconfsize / 4)) = codeval2;
                            gameconfsize += 4;
                            DCFlushRange((void *) (gameconf + (gameconfsize / 4) - 5), 20);
                        }
                    }
                    if (strncasecmp("searchandpoke", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer)
                            == 13)
                    {
                        ret = sscanf((char *) (tempgameconf + i), "( %x%n", &codeval, &tempoffset);
                        if (ret == 1)
                        {
                            gameconfsize += 4;
                            temp = 0;
                            while (ret == 1)
                            {
                                *(gameconf + (gameconfsize / 4)) = codeval;
                                gameconfsize += 4;
                                temp++;
                                i += tempoffset;
                                ret = sscanf((char *) (tempgameconf + i), " %x%n", &codeval, &tempoffset);
                            }
                            *(gameconf + (gameconfsize / 4) - temp - 1) = temp;
                            ret = sscanf((char *) (tempgameconf + i), " , %x , %x , %x , %x", &codeaddr, &codeaddr2,
                                    &codeoffset, &codeval2);
                            if (ret == 4)
                            {
                                *(gameconf + (gameconfsize / 4)) = codeaddr;
                                gameconfsize += 4;
                                *(gameconf + (gameconfsize / 4)) = codeaddr2;
                                gameconfsize += 4;
                                *(gameconf + (gameconfsize / 4)) = codeoffset;
                                gameconfsize += 4;
                                *(gameconf + (gameconfsize / 4)) = codeval2;
                                gameconfsize += 4;
                                DCFlushRange((void *) (gameconf + (gameconfsize / 4) - temp - 5), temp * 4 + 20);
                            }
                            else gameconfsize -= temp * 4 + 4;
                        }

                    }
                    /*
                     if (strncasecmp("hook", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 4)
                     {
                     ret = sscanf(tempgameconf + i, "( %x %x %x %x %x %x %x %x", customhook, customhook + 1, customhook + 2, customhook + 3, customhook + 4, customhook + 5, customhook + 6, customhook + 7);
                     if (ret >= 3)
                     {
                     if (hookset != 1)
                     configwarn |= 4;
                     config_bytes[2] = 0x08;
                     customhooksize = ret * 4;
                     }
                     }
                     if (strncasecmp("002fix", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 6)
                     {
                     ret = sscanf(tempgameconf + i, " = %u", &temp);
                     if (ret == 1)
                     if (temp >= 0 && temp <= 0x1)
                     fakeiosversion = temp;
                     }
                     if (strncasecmp("switchios", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 9)
                     {
                     ret = sscanf(tempgameconf + i, " = %u", &temp);
                     if (ret == 1)
                     if (temp >= 0 && temp <= 1)
                     willswitchios = temp;
                     }
                     if (strncasecmp("videomode", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 9)
                     {
                     ret = sscanf(tempgameconf + i, " = %u", &temp);
                     if (ret == 1)
                     {
                     if (temp == 0)
                     {
                     if (config_bytes[1] != 0x00)
                     configwarn |= 1;
                     config_bytes[1] = 0x00;
                     }
                     else if (temp == 1)
                     {
                     if (config_bytes[1] != 0x03)
                     configwarn |= 1;
                     config_bytes[1] = 0x03;
                     }
                     else if (temp == 2)
                     {
                     if (config_bytes[1] != 0x01)
                     configwarn |= 1;
                     config_bytes[1] = 0x01;
                     }
                     else if (temp == 3)
                     {
                     if (config_bytes[1] != 0x02)
                     configwarn |= 1;
                     config_bytes[1] = 0x02;
                     }
                     }
                     }
                     if (strncasecmp("language", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 8)
                     {
                     ret = sscanf(tempgameconf + i, " = %u", &temp);
                     if (ret == 1)
                     {
                     if (temp == 0)
                     {
                     if (config_bytes[0] != 0xCD)
                     configwarn |= 2;
                     config_bytes[0] = 0xCD;
                     }
                     else if (temp > 0 && temp <= 10)
                     {
                     if (config_bytes[0] != temp-1)
                     configwarn |= 2;
                     config_bytes[0] = temp-1;
                     }
                     }
                     }
                     if (strncasecmp("diagnostic", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 10)
                     {
                     ret = sscanf(tempgameconf + i, " = %u", &temp);
                     if (ret == 1)
                     {
                     if (temp == 0 || temp == 1)
                     diagcreate = temp;
                     }
                     }
                     if (strncasecmp("vidtv", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 5)
                     {
                     ret = sscanf(tempgameconf + i, " = %u", &temp);
                     if (ret == 1)
                     if (temp >= 0 && temp <= 1)
                     vipatchon = temp;
                     }
                     if (strncasecmp("fwritepatch", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 11)
                     {
                     ret = sscanf(tempgameconf + i, " = %u", &temp);
                     if (ret == 1)
                     if (temp >= 0 && temp <= 1)
                     applyfwritepatch = temp;
                     }
                     if (strncasecmp("dumpmaindol", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 11)
                     {
                     ret = sscanf(tempgameconf + i, " = %u", &temp);
                     if (ret == 1)
                     if (temp >= 0 && temp <= 1)
                     dumpmaindol = temp;
                     }
                     */
                }
                /*else
                 {

                 if (strncasecmp("autoboot", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 8)
                 {
                 ret = sscanf(tempgameconf + i, " = %u", &temp);
                 if (ret == 1)
                 if (temp >= 0 && temp <= 1)
                 autoboot = temp;
                 }
                 if (strncasecmp("autobootwait", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 12)
                 {
                 ret = sscanf(tempgameconf + i, " = %u", &temp);
                 if (ret == 1)
                 if (temp >= 0 && temp <= 255)
                 autobootwait = temp;
                 }
                 if (strncasecmp("autoboothbc", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 11)
                 {
                 ret = sscanf(tempgameconf + i, " = %u", &temp);
                 if (ret == 1)
                 if (temp >= 0 && temp <= 1)
                 autoboothbc = temp;
                 }
                 if (strncasecmp("autobootocarina", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 15)
                 {
                 ret = sscanf(tempgameconf + i, " = %u", &temp);
                 if (ret == 1)
                 if (temp >= 0 && temp <= 1)
                 config_bytes[4] = temp;
                 }
                 if (strncasecmp("autobootdebugger", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 16)
                 {
                 ret = sscanf(tempgameconf + i, " = %u", &temp);
                 if (ret == 1)
                 if (temp >= 0 && temp <= 1)
                 config_bytes[7] = temp;
                 }
                 if (strncasecmp("rebootermenuitem", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 16)
                 {
                 ret = sscanf(tempgameconf + i, " = %u", &temp);
                 if (ret == 1)
                 if (temp >= 0 && temp <= 1)
                 rebooterasmenuitem = temp;
                 }
                 if (strncasecmp("startupios", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 10)
                 {
                 ret = sscanf(tempgameconf + i, " = %u", &temp);
                 if (ret == 1)
                 if (temp >= 0 && temp <= 255)
                 {
                 sdio_Shutdown();
                 IOS_ReloadIOS(temp);
                 detectIOScapabilities();
                 sd_init();
                 startupiosloaded = 1;
                 }
                 }

                 }*/
                if (tempgameconf[i] != ':')
                {
                    while ((i != tempgameconfsize) && (tempgameconf[i] != 10 && tempgameconf[i] != 13))
                        i++;
                    if (i != tempgameconfsize) i++;
                }
            }
            if (i != tempgameconfsize) while ((tempgameconf[i] != 10 && tempgameconf[i] != 13) && (i != 0))
                i--;
        }
    }

    free(tempgameconf);
    //tempcodelist = ((u8 *) gameconf) + gameconfsize;
}

int ocarina_load_code(u8 *id)
{
    if (debuggerselect == 0x00)
        codelist = (u8 *) 0x800022A8;
    else codelist = (u8 *) 0x800028B8;
    codelistend = (u8 *) 0x80003000;

    app_loadgameconfig((char *) id);

    char filepath[150];

    gprintf("Ocarina: Searching codes...");
    gprintf("\n");

    sprintf(filepath, "%s%s", CheatFilepath, (char *) id);
    filepath[strlen(CheatFilepath) + 6] = 0x2E;
    filepath[strlen(CheatFilepath) + 7] = 0x67;
    filepath[strlen(CheatFilepath) + 8] = 0x63;
    filepath[strlen(CheatFilepath) + 9] = 0x74;
    filepath[strlen(CheatFilepath) + 10] = 0;

    FILE * fp = fopen(filepath, "rb");
    if (!fp)
    {
        gprintf("Ocarina: No codes found");
        printf("\n");
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    u32 filesize = ftell(fp);
    rewind(fp);

    code_buf = (u8*) malloc(filesize);
    if (!code_buf)
    {
        gprintf("Ocarina: No codes found\n");
        fclose(fp);
        return 0;
    }

    code_size = fread(code_buf, 1, filesize, fp);

    fclose(fp);

    if (code_size <= 0)
    {
        gprintf("Ocarina: could not read file.\n");
        free(code_buf);
        code_buf = NULL;
        code_size = 0;
        return 0;
    }

    if (code_size > (s32) codelistend - (s32) codelist)
    {
        gprintf("Ocarina: Too many codes found\n");
        free(code_buf);
        code_buf = NULL;
        code_size = 0;
        return 0;
    }

    gprintf("Ocarina: Codes found.\n");

    return code_size;
}

//---------------------------------------------------------------------------------
void app_pokevalues()
//---------------------------------------------------------------------------------
{
    u32 i, *codeaddr, *codeaddr2, *addrfound = NULL;

    if (gameconfsize != 0)
    {
        for (i = 0; i < gameconfsize / 4; i++)
        {
            if (*(gameconf + i) == 0)
            {
                if (((u32 *) (*(gameconf + i + 1))) == NULL || *((u32 *) (*(gameconf + i + 1))) == *(gameconf + i + 2))
                {
                    *((u32 *) (*(gameconf + i + 3))) = *(gameconf + i + 4);
                    DCFlushRange((void *) *(gameconf + i + 3), 4);
                }
                i += 4;
            }
            else
            {
                codeaddr = (u32 *) *(gameconf + i + *(gameconf + i) + 1);
                codeaddr2 = (u32 *) *(gameconf + i + *(gameconf + i) + 2);
                if (codeaddr == 0 && addrfound != NULL)
                    codeaddr = addrfound;
                else if (codeaddr == 0 && codeaddr2 != 0)
                    codeaddr = (u32 *) ((((u32) codeaddr2) >> 28) << 28);
                else if (codeaddr == 0 && codeaddr2 == 0)
                {
                    i += *(gameconf + i) + 4;
                    continue;
                }
                if (codeaddr2 == 0) codeaddr2 = codeaddr + *(gameconf + i);
                addrfound = NULL;
                while (codeaddr <= (codeaddr2 - *(gameconf + i)))
                {
                    if (memcmp(codeaddr, gameconf + i + 1, (*(gameconf + i)) * 4) == 0)
                    {
                        *(codeaddr + ((*(gameconf + i + *(gameconf + i) + 3)) / 4)) = *(gameconf + i + *(gameconf + i)
                                + 4);
                        if (addrfound == NULL) addrfound = codeaddr;
                    }
                    codeaddr++;
                }
                i += *(gameconf + i) + 4;
            }
        }
    }
}

//---------------------------------------------------------------------------------
void load_handler()
//---------------------------------------------------------------------------------
{
    if (hooktype != 0x00)
    {
        if (debuggerselect == 0x01)
        {
            /*switch(gecko_channel)
             {
             case 0: // Slot A

             memset((void*)0x80001800,0,codehandlerslota_size);
             memcpy((void*)0x80001800,codehandlerslota,codehandlerslota_size);
             if (pausedstartoption == 0x01)
             *(u32*)0x80002798 = 1;
             memcpy((void*)0x80001CDE, &codelist, 2);
             memcpy((void*)0x80001CE2, ((u8*) &codelist) + 2, 2);
             memcpy((void*)0x80001F7E, &codelist, 2);
             memcpy((void*)0x80001F82, ((u8*) &codelist) + 2, 2);
             DCFlushRange((void*)0x80001800,codehandlerslota_size);
             break;

             case 1: // slot B
             */
            memset((void*) 0x80001800, 0, codehandler_size);
            memcpy((void*) 0x80001800, codehandler, codehandler_size);
            //TODO for oggzee: Consider adding an option for paused start, debugging related
            //if (pausedstartoption == 0x01)
            //  *(u32*)0x80002798 = 1;
            memcpy((void*) 0x80001CDE, &codelist, 2);
            memcpy((void*) 0x80001CE2, ((u8*) &codelist) + 2, 2);
            memcpy((void*) 0x80001F5A, &codelist, 2);
            memcpy((void*) 0x80001F5E, ((u8*) &codelist) + 2, 2);
            DCFlushRange((void*) 0x80001800, codehandler_size);
            /*  break;

             case 2:
             memset((void*)0x80001800,0,codehandler_size);
             memcpy((void*)0x80001800,codehandler,codehandler_size);
             if (pausedstartoption == 0x01)
             *(u32*)0x80002798 = 1;
             memcpy((void*)0x80001CDE, &codelist, 2);
             memcpy((void*)0x80001CE2, ((u8*) &codelist) + 2, 2);
             memcpy((void*)0x80001F5A, &codelist, 2);
             memcpy((void*)0x80001F5E, ((u8*) &codelist) + 2, 2);
             DCFlushRange((void*)0x80001800,codehandler_size);
             break;
             }*/
        }
        else
        {
            memset((void*) 0x80001800, 0, codehandleronly_size);
            memcpy((void*) 0x80001800, codehandleronly, codehandleronly_size);
            memcpy((void*) 0x80001906, &codelist, 2);
            memcpy((void*) 0x8000190A, ((u8*) &codelist) + 2, 2);
            DCFlushRange((void*) 0x80001800, codehandleronly_size);
        }
        // Load multidol handler
        memset((void*) 0x80001000, 0, multidol_size);
        memcpy((void*) 0x80001000, multidol, multidol_size);
        DCFlushRange((void*) 0x80001000, multidol_size);
        switch (hooktype)
        {
            case 0x01:
                memcpy((void*) 0x8000119C, viwiihooks, 12);
                memcpy((void*) 0x80001198, viwiihooks + 3, 4);
                break;
            case 0x02:
                memcpy((void*) 0x8000119C, kpadhooks, 12);
                memcpy((void*) 0x80001198, kpadhooks + 3, 4);
                break;
            case 0x03:
                memcpy((void*) 0x8000119C, joypadhooks, 12);
                memcpy((void*) 0x80001198, joypadhooks + 3, 4);
                break;
            case 0x04:
                memcpy((void*) 0x8000119C, gxdrawhooks, 12);
                memcpy((void*) 0x80001198, gxdrawhooks + 3, 4);
                break;
            case 0x05:
                memcpy((void*) 0x8000119C, gxflushhooks, 12);
                memcpy((void*) 0x80001198, gxflushhooks + 3, 4);
                break;
            case 0x06:
                memcpy((void*) 0x8000119C, ossleepthreadhooks, 12);
                memcpy((void*) 0x80001198, ossleepthreadhooks + 3, 4);
                break;
            case 0x07:
                memcpy((void*) 0x8000119C, axnextframehooks, 12);
                memcpy((void*) 0x80001198, axnextframehooks + 3, 4);
                break;
            case 0x08:
                //if (customhooksize == 16)
                //{
                //  memcpy((void*)0x8000119C,customhook,12);
                //  memcpy((void*)0x80001198,customhook+3,4);
                //}
                break;
            case 0x09:
                //memcpy((void*)0x8000119C,wpadbuttonsdownhooks,12);
                //memcpy((void*)0x80001198,wpadbuttonsdownhooks+3,4);
                break;
            case 0x0A:
                //memcpy((void*)0x8000119C,wpadbuttonsdown2hooks,12);
                //memcpy((void*)0x80001198,wpadbuttonsdown2hooks+3,4);
                break;
        }
        DCFlushRange((void*) 0x80001198, 16);
    }
    memcpy((void *) 0x80001800, (void*) 0x80000000, 6);
}

int ocarina_do_code()
{
    if (!code_buf)
    {
        return 0;
    }

    memset((void *) 0x80001800, 0, 0x1800);

    load_handler();
    memset(codelist, 0, (u32) codelistend - (u32) codelist);

    //Copy the codes
    if (code_size > 0)
    {
        memcpy(codelist, code_buf, code_size);
        DCFlushRange(codelist, (u32) codelistend - (u32) codelist);
        free(code_buf);
        code_buf = NULL;
    }

    // TODO What's this???
    // enable flag
    //*(vu8*)0x80001807 = 0x01;

    //This needs to be done after loading the .dol into memory
    app_pokevalues();

    // hooks are patched in dogamehooks()
    return 1;
}

u32 do_bca_code(u8 *gameid)
{
    if (!BCAFilepath) return 0;

    if (IOS_GetVersion() == 222 || IOS_GetVersion() == 223)
    {
        FILE *fp;
        u32 filesize;
        char filepath[150];
        memset(filepath, 0, 150);
        u8 bcaCode[64] ATTRIBUTE_ALIGN( 32 );

        sprintf(filepath, "%s%6s", BCAFilepath, gameid);
        filepath[strlen(BCAFilepath) + 6] = '.';
        filepath[strlen(BCAFilepath) + 7] = 'b';
        filepath[strlen(BCAFilepath) + 8] = 'c';
        filepath[strlen(BCAFilepath) + 9] = 'a';

        fp = fopen(filepath, "rb");
        if (!fp)
        {
            memset(filepath, 0, 150);
            sprintf(filepath, "%s%3s", BCAFilepath, gameid + 1);
            filepath[strlen(BCAFilepath) + 3] = '.';
            filepath[strlen(BCAFilepath) + 4] = 'b';
            filepath[strlen(BCAFilepath) + 5] = 'c';
            filepath[strlen(BCAFilepath) + 6] = 'a';
            fp = fopen(filepath, "rb");

            if (!fp)
            {
                // Set default bcaCode
                memset(bcaCode, 0, 64);
                bcaCode[0x33] = 1;
            }
        }

        if (fp)
        {
            u32 ret = 0;

            fseek(fp, 0, SEEK_END);
            filesize = ftell(fp);

            if (filesize == 64)
            {
                fseek(fp, 0, SEEK_SET);
                ret = fread(bcaCode, 1, 64, fp);
            }
            fclose(fp);

            if (ret != 64)
            {
                // Set default bcaCode
                memset(bcaCode, 0, 64);
                bcaCode[0x33] = 1;
            }
        }

		Set_DIP_BCA_Datas(bcaCode);
    }
    return 0;
}
