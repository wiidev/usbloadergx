/*  patchmii_core -- low-level functions to handle the downloading, patching
    and installation of updates on the Wii

    Copyright (C) 2008 bushing / hackmii.com
    Copyright (C) 2008 WiiGator
	Copyright (C) 2009 Hermes

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 2.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <network.h>
#include <sys/errno.h>
#include <fat.h>
#include <sys/stat.h>
#include <wiiuse/wpad.h>

#include "patchmii_core.h"
#include "sha1.h"
#include "debug.h"
#include "http.h"
#include "haxx_certs.h"
#include "runtime_ios_patch.h"

//#define  _DEBUG_PRINTF_H_ 1

#include "debug_printf.h"

/*
NOTE: i don't necessary
#ifdef ADD_DIP_PLUGIN
#include "add_dip_plugin.h"
#include "patch_handle_di_cmd_raw.h"
#endif
*/

char *str_trace="";

#define VERSION "5.1"

#define INPUT_TITLEID_H 1
int INPUT_TITLEID_L= 36;
int INPUT_VERSION= 1042;

#define OUTPUT_TITLEID_H 1
int OUTPUT_TITLEID_L=222;

#define OUTPUT_VERSION 65535


#define ALIGN(a,b) ((((a)+(b)-1)/(b))*(b))
#define round_up(x,n) (-(-(x) & -(n)))

static u8 ESCommonKey[16] = { 0xeb, 0xe4, 0x2a, 0x22, 0x5e, 0x85, 0x93, 0xe4, 0x48, 0xd9, 0xc5, 0x45, 0x73, 0x81, 0xaa, 0xf7 };

int http_status = 0;
int useSd = 1;
int tmd_dirty = 0, tik_dirty = 0;


u32 DIP_patch1_pos=0x6800;
u32 DIP_DVD_enable_orig_pos1=0x964;
u32 DIP_DVD_enable_orig_pos2=0x9F0;

u32 DIP_handle_di_cmd=0x112c;
u8 *patch_handle_di_cmd=NULL;
int len_patch_handle_di_cmd=0;

u32 DIP_handle_di_cmd_reentry=0x8248;
u32 len_handle_di_cmd_reentry=0;
u8 *handle_di_cmd_reentry=NULL;

//u32 ES_ioctvl_patch_pos=0x12ab0;
//u8 *ES_patch_ioctvl=NULL;



void IRQS_patchs(unsigned char *p, int len)
{
int n;

for(n=0;n<len-16;n++) 
	{
	
	// referencia de ejecutable
	if(!memcmp((void *) p, "Lockdown TLB", 12))
		{
		int m;

		// search software_IRQ table

		for(m=0;m<255;m++)
			{
			if(p[m]==0xff && p[m+1]==0xff && p[m+4]==0xff && p[m+5]==0xff &&  p[m+8]==0xff && p[m+9]==0xff)
				{
				u16 data=(p[m+2]<<8) | p[m+3];
				data+=0xa; // skip check code
                p[m+2]=data>>8; // patch IRQ 4
				p[m+3]=data;
                
				m+=5*4;
				p[m+0]=0x13;  // IRQ 9 (unused by system) vector patched (used to call function in system mode)
				p[m+1]=0x8c;
				p[m+2]=0x00;
				p[m+3]=0x08+1;
				
				printf("Software IRQ 4 and IRQ 9 patched\n");
				return;
				}
			
			}
		return;
		}
	p++;
	}
return;
}


u8 patch_handle_di_cmd36[12] = {
	0x4B, 0x01, 0x68, 0x1B, 0x47, 0x18, 0x00, 0x00,/*addr to get handle_di_cmd*/ 0x20, 0x20, 0x90, 0x40 
};

// handle_di_cmd_reentry= 0x20209030 (default)
u8 handle_di_cmd_reentry36[24] = {
	0x20, 0x20, 0x90, 0x44+1,
	0xB5, 0xF0, 0x46, 0x5F, 0x46, 0x56, 0x46, 0x4D, 0x46, 0x44, 0xB4, 0xF0, 0x4B, 0x00, 0x47, 0x18, 
	/* handle_di_cmd_reentry */ 0x20, 0x20, 0x10, 0x10+1 // (Thumb)
};

u8 patch_handle_di_cmd37[12] = {
	0x4B, 0x01, 0x68, 0x1B, 0x47, 0x18, 0x00, 0x00,/*addr to get handle_di_cmd*/ 0x20, 0x20, 0x90, 0x30 
};

// handle_di_cmd_reentry= 0x20209030 (default)
u8 handle_di_cmd_reentry37[24] = {
	0x20, 0x20, 0x90, 0x34+1,
	0xB5, 0xF0, 0x46, 0x5F, 0x46, 0x56, 0x46, 0x4D, 0x46, 0x44, 0xB4, 0xF0, 0x4B, 0x00, 0x47, 0x18, 
	/* handle_di_cmd_reentry */ 0x20, 0x20, 0x0f, 0x04+1 // (Thumb)
};


u8 patch_handle_di_cmd38[12] = {
	0x4B, 0x01, 0x68, 0x1B, 0x47, 0x18, 0x00, 0x00,/*addr to get handle_di_cmd*/ 0x20, 0x20, 0x80, 0x30
};

// handle_di_cmd_reentry= 0x20208030 (default)
u8 handle_di_cmd_reentry38[24] = {
	0x20, 0x20, 0x80, 0x34+1,
	0xB5, 0xF0, 0x46, 0x5F, 0x46, 0x56, 0x46, 0x4D, 0x46, 0x44, 0xB4, 0xF0, 0x4B, 0x00, 0x47, 0x18, 
	/* handle_di_cmd_reentry */  0x20, 0x20, 0x0D, 0x38+1 // (Thumb)
};


u8 patch_handle_di_cmd60[12] = {
	0x4B, 0x01, 0x68, 0x1B, 0x47, 0x18, 0x00, 0x00,/*addr to get handle_di_cmd*/ 0x20, 0x20, 0x80, 0x30
};

// handle_di_cmd_reentry= 0x20208030 (default)
u8 handle_di_cmd_reentry60[24] = {
	0x20, 0x20, 0x80, 0x34+1,
	0xB5, 0xF0, 0x46, 0x5F, 0x46, 0x56, 0x46, 0x4D, 0x46, 0x44, 0xB4, 0xF0, 0x4B, 0x00, 0x47, 0x18, 
	/* handle_di_cmd_reentry */  0x20, 0x20, 0x0D, 0x38+1 // (Thumb)
};


void adjust_patch(int ios)
{

switch(ios)
	{
	case 36:
		DIP_patch1_pos=0x6800;
		DIP_DVD_enable_orig_pos1=0x964;
		DIP_DVD_enable_orig_pos2=0x9F0;
		DIP_handle_di_cmd=0x112c;
		patch_handle_di_cmd=patch_handle_di_cmd36;
		len_patch_handle_di_cmd=sizeof(patch_handle_di_cmd36);
		DIP_handle_di_cmd_reentry=0x8248;
		handle_di_cmd_reentry=handle_di_cmd_reentry36;
		len_handle_di_cmd_reentry=sizeof(handle_di_cmd_reentry36);
		break;

	case 38:
		DIP_patch1_pos=0x6494;
		DIP_DVD_enable_orig_pos1=0x68c;
		DIP_DVD_enable_orig_pos2= 0x718;
		DIP_handle_di_cmd= 0xe54;
		patch_handle_di_cmd=patch_handle_di_cmd38;
		len_patch_handle_di_cmd=sizeof(patch_handle_di_cmd38);
		DIP_handle_di_cmd_reentry=0x7ecc;
		handle_di_cmd_reentry=handle_di_cmd_reentry38;
		len_handle_di_cmd_reentry=sizeof(handle_di_cmd_reentry38);
		break;

	case 37:
	   
		DIP_patch1_pos=0x6768;
		DIP_DVD_enable_orig_pos1=0x6e4;
		DIP_DVD_enable_orig_pos2=0x774;

		DIP_handle_di_cmd=0x1020;
		patch_handle_di_cmd=patch_handle_di_cmd37;
		len_patch_handle_di_cmd=sizeof(patch_handle_di_cmd37);
		DIP_handle_di_cmd_reentry=0x81e0;
		handle_di_cmd_reentry=handle_di_cmd_reentry37;
		len_handle_di_cmd_reentry=sizeof(handle_di_cmd_reentry37);
		break;

	case 57:

		// use IOS57 DIP and ES: NOTE DIP IOS60 is equal to this IOS57 version an ES use the same patch
	
	    /* old 5404
		DIP_patch1_pos=0x671c;
		DIP_DVD_enable_orig_pos1=0x6e4;
		DIP_DVD_enable_orig_pos2= 0x774;
		DIP_handle_di_cmd= 0x1020;
		patch_handle_di_cmd=patch_handle_di_cmd60;
		len_patch_handle_di_cmd=sizeof(patch_handle_di_cmd60);
		DIP_handle_di_cmd_reentry=0x8058;
		handle_di_cmd_reentry=handle_di_cmd_reentry60;
		len_handle_di_cmd_reentry=sizeof(handle_di_cmd_reentry60);
		*/
		DIP_patch1_pos=0x680c;
		DIP_DVD_enable_orig_pos1=0x6e4;
		DIP_DVD_enable_orig_pos2= 0x774;
		DIP_handle_di_cmd= 0x1020;
		patch_handle_di_cmd=patch_handle_di_cmd60;
		len_patch_handle_di_cmd=sizeof(patch_handle_di_cmd60);
		DIP_handle_di_cmd_reentry=0x8148;
		handle_di_cmd_reentry=handle_di_cmd_reentry60; // use the same patch from IOS 60
		len_handle_di_cmd_reentry=sizeof(handle_di_cmd_reentry60);
		break;

	case 60:
		// use IOS60 DIP and ES
	
		DIP_patch1_pos=0x671c;
		DIP_DVD_enable_orig_pos1=0x6e4;
		DIP_DVD_enable_orig_pos2= 0x774;
		DIP_handle_di_cmd= 0x1020;
		patch_handle_di_cmd=patch_handle_di_cmd60;
		len_patch_handle_di_cmd=sizeof(patch_handle_di_cmd60);
		DIP_handle_di_cmd_reentry=0x8058;
		handle_di_cmd_reentry=handle_di_cmd_reentry60;
		len_handle_di_cmd_reentry=sizeof(handle_di_cmd_reentry60);
		break;
	
	default:
		error_debug_printf("Unsupported IOS");
		exit(0);
		break;
	}

}



u8 DIP_orig1[] =  { 0x00, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
u8 DIP_patch1[] = { 0x7e, 0xd4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

u8 DIP_DVD_enable_orig[] = { 0x20, 0x01 };
u8 DIP_DVD_enable_patch[] = { 0x20, 0x00 };
u8 DIP_handle_di_cmd_orig[] = { 0xb5, 0xf0, 0x46, 0x5f, 0x46, 0x56, 0x46, 0x4d, 0x46, 0x44, 0xb4, 0xf0 };

static int patchmii(void);

int replace_ios_modules(u8 **decrypted_buf,  u32 *content_size);
int add_custom_modules(tmd *p_tmd);


void debug_printf(const char *fmt, ...) {
  char buf[1024];
  int len;
  va_list ap;
  usb_flush(1);
  va_start(ap, fmt);
  len = vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  if (len <= 0 || len > sizeof(buf)) printf("\33[41mError: len = %d\33[40m\n", len);
  else usb_sendbuffer(1, buf, len);
  printf("%s",buf);
}

void error_debug_printf(const char *fmt, ...) {
  char buf[1024];
  int len;
  va_list ap;
  usb_flush(1);
  va_start(ap, fmt);
  len = vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  if (len <= 0 || len > sizeof(buf)) printf("\33[41mError: len = %d\33[40m\n", len);
  else usb_sendbuffer(1, buf, len);
  printf("\33[41m%s\33[40m\n",buf);
}

char ascii(char s) {
  if(s < 0x20) return '.';
  if(s > 0x7E) return '.';
  return s;
}

void hexdump(void *d, int len) {
  u8 *data;
  int i, off;
  data = (u8*)d;
  for (off=0; off<len; off += 16) {
    debug_printf("%08x  ",off);
    for(i=0; i<16; i++)
      if((i+off)>=len) debug_printf("   ");
      else debug_printf("%02x ",data[off+i]);

    debug_printf(" ");
    for(i=0; i<16; i++)
      if((i+off)>=len) debug_printf(" ");
      else debug_printf("%c",ascii(data[off+i]));
    debug_printf("\n");
  }
}

char *spinner_chars="/-\\|";
int spin = 0;

void spinner(void) {
  printf("\b%c", spinner_chars[spin++]);
  if(!spinner_chars[spin]) spin=0;
}

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void printvers(void) {
  debug_printf("IOS Version: %08x\n", *((u32*)0xC0003140));
}

void console_setup(void) {
  VIDEO_Init();
  PAD_Init();
  WPAD_Init();
  
  rmode = VIDEO_GetPreferredMode(NULL);

  xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
  VIDEO_ClearFrameBuffer(rmode,xfb,COLOR_BLACK);
  VIDEO_Configure(rmode);
  VIDEO_SetNextFramebuffer(xfb);
  VIDEO_SetBlack(FALSE);
  VIDEO_Flush();
  VIDEO_WaitVSync();
  if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
  CON_InitEx(rmode,20,30,rmode->fbWidth - 40,rmode->xfbHeight - 60);
}

static char buf[128];

int get_nus_object(u32 titleid1, u32 titleid2, u32 version, char *content, u8 **outbuf, u32 *outlen) {
  
  int retval;
  u32 http_status;
  static int netInit = 0;

FILE *fd;
	
  if (useSd) {
    

	// NUS Downloader format
	snprintf(buf, 128, "sd:/ios/%08x%08xv%d/%s", titleid1, titleid2, version, content);
	fd = fopen(buf, "rb");
	if(!fd)
	  {
	  snprintf(buf, 128, "sd:/%08x/%08x/v%d/%s", titleid1, titleid2, version, content);
	  fd = fopen(buf, "rb");
	  }

	if (!fd) {
		debug_printf("from Internet: ");
	} else {
		debug_printf("from SD: ");
		fseek(fd, 0, SEEK_END);
		*outlen = ftell(fd);
		fseek(fd, 0, SEEK_SET);

		*outbuf = malloc(*outlen);
		if (*outbuf == NULL) {
			error_debug_printf("Out of memory size %d", *outlen);
			return 2;
		}

		if (fread(*outbuf, *outlen, 1, fd) != 1) {
			fclose(fd);
			return 3;
		} else {
			fclose(fd);
			return 0;
		}
	}
  }
  if (!netInit)
  {
  time_t  rel_time=time(NULL);

  	printf("Initializing network."); fflush(stdout);
  	while (1) {
  		retval = net_init ();
 		if (retval < 0) {
			if (retval != -EAGAIN) {
				error_debug_printf ("net_init failed: %d", retval);
				return 4;
			}
    	}
		if (!retval) break;
		usleep(100000);
		printf("."); fflush(stdout);
		if((time(NULL)-rel_time)>15)
			{
				error_debug_printf ("net_init failed: %d", retval);
				return 4;
			}
  	}
    sleep(1);
  	printf("Done!\n");
    netInit = 1;
  }
  snprintf(buf, 128, "http://nus.cdn.shop.wii.com/ccs/download/%08x%08x/%s",
	   titleid1, titleid2, content);

  debug_printf("\nwget -O sd:/ios/%08x%08xv%d/%s %s\n", titleid1, titleid2, version, content,buf);

	{int retry=10;
	while(1)
		{
		  retval = http_request(buf, (u32) (1 << 31));
		  if (!retval) {
			  retry--;
			error_debug_printf("Error making http request");
			sleep(1);
			if(retry<0) return 1;
		  }
		else break;
		}
	}
  retval = http_get_result(&http_status, outbuf, outlen);
	//snprintf(buf, 128, "sd:/%08x/%08x/v%d/%s", titleid1, titleid2, version, content);	
	snprintf(buf, 128, "sd:/ios/%08x%08xv%d/%s", titleid1, titleid2, version, content);

	if (useSd)
	{
	fd = fopen(buf, "wb");
	if (fd) {
			fwrite(*outbuf, *outlen, 1, fd);
			fclose(fd);
			}
	}

  if (((int)*outbuf & 0xF0000000) == 0xF0000000) {

	
	return (int) *outbuf;
  }

  return 0;
}

void decrypt_buffer(u16 index, u8 *source, u8 *dest, u32 len) {
  static u8 iv[16];
  if (!source) {
	error_debug_printf("decrypt_buffer: invalid source paramater");
	exit(1);
  }
  if (!dest) {
	error_debug_printf("decrypt_buffer: invalid dest paramater");
	exit(1);
  }

  memset(iv, 0, 16);
  memcpy(iv, &index, 2);
  aes_decrypt(iv, source, dest, len);
}

static u8 encrypt_iv[16];
void set_encrypt_iv(u16 index) {
  memset(encrypt_iv, 0, 16);
  memcpy(encrypt_iv, &index, 2);
}
  
void encrypt_buffer(u8 *source, u8 *dest, u32 len) {
  aes_encrypt(encrypt_iv, source, dest, len);
}

int create_temp_dir(void) {
  int retval;
  retval = ISFS_CreateDir ("/tmp/patchmii", 0, 3, 1, 1);

  if (retval) error_debug_printf("ISFS_CreateDir(/tmp/patchmii) returned %d", retval);
  return retval;
}

u32 save_nus_object (u16 index, u8 *buf, u32 size) {
  char filename[256];
  static u8 bounce_buf[1024] ATTRIBUTE_ALIGN(0x20);
  u32 i;

  int retval, fd;
  snprintf(filename, sizeof(filename), "/tmp/patchmii/%08x", index);
  
  retval = ISFS_CreateFile (filename, 0, 3, 1, 1);

  if (retval != ISFS_OK) {
    error_debug_printf("ISFS_CreateFile(%s) returned %d", filename, retval);
    return retval;
  }
  
  fd = ISFS_Open (filename, ISFS_ACCESS_WRITE);

  if (fd < 0) {
    error_debug_printf("ISFS_OpenFile(%s) returned %d", filename, fd);
    return retval;
  }

  for (i=0; i<size;) {
    u32 numbytes = ((size-i) < 1024)?size-i:1024;
    memcpy(bounce_buf, buf+i, numbytes);
    retval = ISFS_Write(fd, bounce_buf, numbytes);
    if (retval < 0) {
      error_debug_printf("ISFS_Write(%d, %p, %d) returned %d at offset %d", 
		   fd, bounce_buf, numbytes, retval, i);
      ISFS_Close(fd);
      return retval;
    }
    i += retval;
  }
  ISFS_Close(fd);
  return size;
}

s32 install_nus_object (tmd *p_tmd, u16 index) {
  char filename[256];
  static u8 bounce_buf1[1024] ATTRIBUTE_ALIGN(0x20);
  static u8 bounce_buf2[1024] ATTRIBUTE_ALIGN(0x20);
  u32 i;
  const tmd_content *p_cr = TMD_CONTENTS(p_tmd);
  int rindex = p_cr[index].index;
  //  debug_printf("install_nus_object(%p, %lu)", p_tmd, rindex);
  
  int retval, fd, cfd, ret;
  snprintf(filename, sizeof(filename), "/tmp/patchmii/%08x", p_cr[index].cid);
  
  fd = ISFS_Open (filename, ISFS_ACCESS_READ);
  
  if (fd < 0) {
    error_debug_printf("ISFS_OpenFile(%s) returned %d", filename, fd);
    return fd;
  }
  set_encrypt_iv(rindex);
  //  debug_printf("ES_AddContentStart(%016llx, %x)\n", p_tmd->title_id, rindex);

  cfd = ES_AddContentStart(p_tmd->title_id, p_cr[index].cid);
  if(cfd < 0) {
    error_debug_printf(":\nES_AddContentStart(%016llx, %x) failed: %d",p_tmd->title_id, index, cfd);
    ES_AddTitleCancel();
    return -1;
  }
  debug_printf(" (cfd %d): ",cfd);
  for (i=0; i<p_cr[index].size;) {
    u32 numbytes = ((p_cr[index].size-i) < 1024)?p_cr[index].size-i:1024;
    numbytes = ALIGN(numbytes, 32);
    retval = ISFS_Read(fd, bounce_buf1, numbytes);
    if (retval < 0) {
      error_debug_printf("ISFS_Read(%d, %p, %d) returned %d at offset %d", 
		   fd, bounce_buf1, numbytes, retval, i);
      ES_AddContentFinish(cfd);
      ES_AddTitleCancel();
      ISFS_Close(fd);
      return retval;
    }
    
    encrypt_buffer(bounce_buf1, bounce_buf2, sizeof(bounce_buf1));
    ret = ES_AddContentData(cfd, bounce_buf2, retval);
    if (ret < 0) {
      error_debug_printf("ES_AddContentData(%d, %p, %d) returned %d", cfd, bounce_buf2, retval, ret);
      ES_AddContentFinish(cfd);
      ES_AddTitleCancel();
      ISFS_Close(fd);
      return ret;
    }
    i += retval;
  }

  debug_printf("  done! (0x%x bytes)\n",i);
  ret = ES_AddContentFinish(cfd);
  if(ret < 0) {
    error_debug_printf("ES_AddContentFinish failed: %d",ret);
    ES_AddTitleCancel();
    ISFS_Close(fd);
    return -1;
  }
  
  ISFS_Close(fd);
  
  return 0;
}

int get_title_key(signed_blob *s_tik, u8 *key) {
  static u8 iv[16] ATTRIBUTE_ALIGN(0x20);
  static u8 keyin[16] ATTRIBUTE_ALIGN(0x20);
  static u8 keyout[16] ATTRIBUTE_ALIGN(0x20);
  int retval = 0;

  const tik *p_tik;
  p_tik = (tik*)SIGNATURE_PAYLOAD(s_tik);
  u8 *enc_key = (u8 *)&p_tik->cipher_title_key;
  memcpy(keyin, enc_key, sizeof keyin);
  memset(keyout, 0, sizeof keyout);
  memset(iv, 0, sizeof iv);
  memcpy(iv, &p_tik->titleid, sizeof p_tik->titleid);
  
  //retval = ES_Decrypt(ES_KEY_COMMON, iv, keyin, sizeof keyin, keyout);
  //if (retval) error_debug_printf("ES_Decrypt returned %d", retval);
  aes_set_key(ESCommonKey);
  aes_decrypt(iv, keyin, keyout, sizeof(keyin));

  memcpy(key, keyout, sizeof keyout);
  return retval;
}

int change_ticket_title_id(signed_blob *s_tik, u32 titleid1, u32 titleid2) {
	static u8 iv[16] ATTRIBUTE_ALIGN(0x20);
	static u8 keyin[16] ATTRIBUTE_ALIGN(0x20);
	static u8 keyout[16] ATTRIBUTE_ALIGN(0x20);
	int retval = 0;

	tik *p_tik;
	p_tik = (tik*)SIGNATURE_PAYLOAD(s_tik);
	u8 *enc_key = (u8 *)&p_tik->cipher_title_key;
	memcpy(keyin, enc_key, sizeof keyin);
	memset(keyout, 0, sizeof keyout);
	memset(iv, 0, sizeof iv);
	memcpy(iv, &p_tik->titleid, sizeof p_tik->titleid);

	//retval = ES_Decrypt(ES_KEY_COMMON, iv, keyin, sizeof keyin, keyout);
  	aes_set_key(ESCommonKey);
	aes_decrypt(iv, keyin, keyout, sizeof(keyin));

	p_tik->titleid = (u64)titleid1 << 32 | (u64)titleid2;
	memset(iv, 0, sizeof iv);
	memcpy(iv, &p_tik->titleid, sizeof p_tik->titleid);
	
	//retval = ES_Encrypt(ES_KEY_COMMON, iv, keyout, sizeof keyout, keyin);
    	//if (retval) error_debug_printf("ES_Decrypt returned %d", retval);
  	aes_set_key(ESCommonKey);
	aes_encrypt(iv, keyout, keyin, sizeof(keyout));

	memcpy(enc_key, keyin, sizeof keyin);
	tik_dirty = 1;

    return retval;
}

void change_tmd_title_id(signed_blob *s_tmd, u32 titleid1, u32 titleid2) {
	tmd *p_tmd;
	u64 title_id = titleid1;
	title_id <<= 32;
	title_id |= titleid2;
	p_tmd = (tmd*)SIGNATURE_PAYLOAD(s_tmd);
	p_tmd->title_id = title_id;
	tmd_dirty = 1;
}

void display_tag(u8 *buf) {
  debug_printf("Firmware version: %s      Builder: %s\n",
	       buf, buf+0x30);
}

void display_ios_tags(u8 *buf, u32 size) {
  u32 i;
  char *ios_version_tag = "$IOSVersion:";

  if (size == 64) {
    display_tag(buf);
    return;
  }

  for (i=0; i<(size-64); i++) {
    if (!strncmp((char *)buf+i, ios_version_tag, 10)) {
      char version_buf[128], *date;
      while (buf[i+strlen(ios_version_tag)] == ' ') i++; // skip spaces
      strlcpy(version_buf, (char *)buf + i + strlen(ios_version_tag), sizeof version_buf);
      date = version_buf;
      strsep(&date, "$");
      date = version_buf;
      strsep(&date, ":");
      debug_printf("%s (%s)\n", version_buf, date);
      i += 64;
    }
  }
}

void print_tmd_summary(const tmd *p_tmd) {
  const tmd_content *p_cr;
  p_cr = TMD_CONTENTS(p_tmd);

  u32 size=0;

  u16 i=0;
  for(i=0;i<p_tmd->num_contents;i++) {
    size += p_cr[i].size;
  }

  debug_printf("Title ID: %016llx\n",p_tmd->title_id);
  debug_printf("Number of parts: %d.  Total size: %uK\n", p_tmd->num_contents, (u32) (size / 1024));
}

void zero_sig(signed_blob *sig) {
  u8 *sig_ptr = (u8 *)sig;
  memset(sig_ptr + 4, 0, SIGNATURE_SIZE(sig)-4);
}

void brute_tmd(tmd *p_tmd) {
  u16 fill;
  for(fill=0; fill<65535; fill++) {
    p_tmd->fill3=fill;
    sha1 hash;
    //    debug_printf("SHA1(%p, %x, %p)\n", p_tmd, TMD_SIZE(p_tmd), hash);
    SHA1((u8 *)p_tmd, TMD_SIZE(p_tmd), hash);
  
    if (hash[0]==0) {
      //      debug_printf("setting fill3 to %04hx\n", fill);
      return;
    }
  }
  error_debug_printf("Unable to fix tmd :(");
  exit(4);
}

void brute_tik(tik *p_tik) {
  u16 fill;
  for(fill=0; fill<65535; fill++) {
    p_tik->padding=fill;
    sha1 hash;
    //    debug_printf("SHA1(%p, %x, %p)\n", p_tmd, TMD_SIZE(p_tmd), hash);
    SHA1((u8 *)p_tik, sizeof(tik), hash);
  
    if (hash[0]==0) return;
  }
  error_debug_printf("Unable to fix tik :(");
  exit(5);
}
    
void forge_tmd(signed_blob *s_tmd) {
  debug_printf("forging tmd sig\n");
  zero_sig(s_tmd);
  brute_tmd(SIGNATURE_PAYLOAD(s_tmd));
}

void forge_tik(signed_blob *s_tik) {
  debug_printf("forging tik sig\n");
  zero_sig(s_tik);
  brute_tik(SIGNATURE_PAYLOAD(s_tik));
}

s32 install_ticket(const signed_blob *s_tik, const signed_blob *s_certs, u32 certs_len) {
  u32 ret;

  debug_printf("Installing ticket...\n");
  ret = ES_AddTicket(s_tik,STD_SIGNED_TIK_SIZE,s_certs,certs_len, NULL, 0);
  if (ret < 0) {
      error_debug_printf("ES_AddTicket failed: %d",ret);
      return ret;
  }
  return 0;
}

s32 install(const signed_blob *s_tmd, const signed_blob *s_certs, u32 certs_len) {
  u32 ret, i;
  tmd *p_tmd = SIGNATURE_PAYLOAD(s_tmd);
  debug_printf("Adding title...\n");

  ret = ES_AddTitleStart(s_tmd, SIGNED_TMD_SIZE(s_tmd), s_certs, certs_len, NULL, 0);

  if(ret < 0) {
    error_debug_printf("ES_AddTitleStart failed: %d",ret);
    ES_AddTitleCancel();
    return ret;
  }

  for(i=0; i<p_tmd->num_contents; i++) {
    debug_printf("Adding content ID %08x", i);
    ret = install_nus_object((tmd *)SIGNATURE_PAYLOAD(s_tmd), i);
    if (ret) return ret;
  }

  ret = ES_AddTitleFinish();
  if(ret < 0) {
    error_debug_printf("ES_AddTitleFinish failed: %d",ret);
    ES_AddTitleCancel();
    return ret;
  }

  printf("Installation complete!\n");
  return 0;

}


void fun_exit()
{
	WPAD_Shutdown();
	sleep(5);
}

u64 *titles = NULL;
u32 num_titles=0;
int ios_index=0;

u8 ios_found[256];

int get_title_list()
{


	u32 len_buf;
	s32 ret;
	int n;

    memset((void *) ios_found,0, 256);

	ret = ES_GetNumTitles(&num_titles);
	if (ret < 0)
		return ret;

	if(num_titles<1) return -1;

	len_buf = round_up((num_titles+1) * sizeof(u64), 32);

	titles = memalign(32, len_buf);
	if (!titles)
		return -1;


	ret = ES_GetTitles(titles, num_titles);
	if (ret < 0)
		goto err;

	n=0;
	while(n<num_titles)
	{
	u32 tidh = (titles[n] >> 32);
	u32 tidl = (titles[n] &  0xFFFFFFFF);

	if ((tidh != 0x1) || (tidl < 3) || (tidl > 255))
		{
		num_titles--;
		memcpy(&titles[n],&titles[n+1], (num_titles-n) * sizeof(u64));
		} 
		else 
			{ios_found[tidl]=1;n++;}
	}

    
return 0;

err:
	
	if (titles) free(titles); titles = NULL;

	return ret;

}

int exit_by_reset=0;

void reset_call() {exit_by_reset=1;}

int main(int argc, char **argv) {
	int rv;
	s32 pressed;
	int selected=0;
	int tick_counter=0;
	
	atexit(fun_exit);
	console_setup();
	printf("This program is a modification of patchmii, and is unsupported and not condoned by the original authors of it.\n");
	printf("The backup loader modification is solely the work of WiiGator.\n");
	printf("This version includes optimizations made by Waninkoko and Hermes\n");
        printf("USB2/wbfs support by Kwiirk\n");
	printf("\n");
	printf("cIOS installer %s by Hermes.\n", VERSION);
	printf("If you get an error, you need to downgrade your Wii first.\n");
	printf("\n");
	printf("USE ON YOUR OWN RISK!\n");
	printf("\n");

	SYS_SetResetCallback(reset_call);
	sleep(2);
	int ahbprot_ok = 0;

	if (HAVE_AHBPROT) {
		printf("Found IOS with disabled AHB Protection!\n");
		printf("\n");
		printf("Applying patches");
		if (IOSPATCH_Apply()) {
			printf(" done!\n");
			ahbprot_ok = 1;
		} else {
			printf("something went wrong.\n");
		}
		sleep(2);
	}	

	if (!ahbprot_ok) {
#if 1

	if(get_title_list()!=0)
	{
		printf("Error getting title list\n");
		return 0;
	}

    ios_index=36;
    if(ios_found[249]) ios_index=249;
	else
	if(ios_found[250]) ios_index=250;
	else
	if(ios_found[222]) ios_index=222;
	else
	if(ios_found[223]) ios_index=223;
	else
	if(ios_found[35]) ios_index=35;
	
	while(!ios_found[ios_index]) {ios_index++;if(ios_index>255) ios_index=0;}
	
	

	while(1)
	{
	printf("\33[2J\n\n\33[46m\33[2K\n\33[2K cIOS Installer %s by Hermes (www.elotrolado.net)\n\33[2K\33[40m\n\n",VERSION);

	printf("  Select IOS with Trucha Bug to use during installation <IOS %i>\n\n", ios_index);
	printf("  %sThe selected IOS must have dev/es patched to work\33[37m\n\n\n", (tick_counter & 32) ? "\33[33m" : "\33[30m");
	printf("  Press LEFT/RIGHT to select other different IOS\n\n");
	printf("  Press A to continue (TAKE THE RISK).\n\n");
	printf("  Press B to abort.\n\n");

		WPAD_ScanPads();
		pressed = WPAD_ButtonsDown(0);

		if(pressed) {
			if (pressed == WPAD_BUTTON_A) {
				break;
			} 
		if (pressed == WPAD_BUTTON_B) {
				printf("Aborted, exiting...\n");
				return 0;
			}
		if (pressed == WPAD_BUTTON_RIGHT)
			{
			do{ios_index++;if(ios_index>255) ios_index=0;} while(!ios_found[ios_index]);
			}
		if (pressed == WPAD_BUTTON_LEFT)
			{
			do{ios_index--;if(ios_index<0) ios_index=255;} while(!ios_found[ios_index]);
			}
		}
		VIDEO_WaitVSync();
		tick_counter++;
		if(exit_by_reset) {
				printf("Aborted, exiting...\n");
				return 0;
			} 
	}
#endif

	printf("\33[42m >>>>>>>>>>>>>>> Reloading IOS %i <<<<<<<<<<<<<<<\33[40m\n", ios_index);
	WPAD_Shutdown();
	sleep(1);
	IOS_ReloadIOS(ios_index);
	sleep(1);

	WPAD_Init();
	selected=1;
}

	while(1)
	{

	printf("\33[2J\n\n\33[46m\33[2K\n\33[2K cIOS Installer (Select Custom IOS)\n\33[2K\33[40m\n\n");
    printf("     %sInstall Custom IOS 202 v%d (Homebrew) \33[40m\n\n", (selected==0 && (tick_counter & 32)) ? ">\33[44m" : " \33[40m", OUTPUT_VERSION);
	printf("     %sInstall Custom IOS 222 v%d (Default)  \33[40m\n\n", (selected==1 && (tick_counter & 32)) ? ">\33[44m" : " \33[40m", OUTPUT_VERSION);
	printf("     %sInstall Custom IOS 223 v%d            \33[40m\n\n", (selected==2 && (tick_counter & 32)) ? ">\33[44m" : " \33[40m", OUTPUT_VERSION);
	printf("     %sInstall Custom IOS 224 v%d            \33[40m\n\n", (selected==3 && (tick_counter & 32)) ? ">\33[44m" : " \33[40m", OUTPUT_VERSION);
	printf("     %sInstall Custom IOS 225 v%d            \33[40m\n\n", (selected==4 && (tick_counter & 32)) ? ">\33[44m" : " \33[40m", OUTPUT_VERSION);

	printf("\n\n     Press A to select or B to Abort\n\n");
	printf("\33[33m Current IOS: %d v%d\33[37m\n\n", *((volatile u32 *) 0x80003140)>>16, *((volatile u32 *) 0x80003140) & 0xffff);

		WPAD_ScanPads();
		pressed = WPAD_ButtonsDown(0);

		if(pressed) {
			if (pressed == WPAD_BUTTON_A) {
				break;
			} 
			if (pressed == WPAD_BUTTON_B) {
				printf("Aborted, exiting...\n");
				return 0;
			} 
			if (pressed == WPAD_BUTTON_UP) {
				selected--;if(selected<0) selected=0;
			} 

			if (pressed == WPAD_BUTTON_DOWN) {
				selected++;if(selected>4) selected=4;
			}
		}
		VIDEO_WaitVSync();
		tick_counter++;
		if(exit_by_reset) {
				printf("Aborted, exiting...\n");
				return 0;
			} 

	}

    switch(selected)
	{
	case 0:
        OUTPUT_TITLEID_L=202;
		break;
	case 1:
        OUTPUT_TITLEID_L=222;
		break;
	case 2:
        OUTPUT_TITLEID_L=223;
		break;
	case 3:
        OUTPUT_TITLEID_L=224;
		break;
	case 4:
        OUTPUT_TITLEID_L=225;
		break;
	}


	selected=0;

	while(1)
	{
	printf("\33[2J\n\n\33[46m\33[2K\n\33[2K cIOS Installer (Select IOS Base)\n\33[2K\33[40m\n\n");
	
	if(OUTPUT_TITLEID_L!=222)
		{
		if(OUTPUT_TITLEID_L==202) printf("     %sUse IOS 38 (Recommended)         \33[40m\n\n", (selected==0 && (tick_counter & 32)) ? ">\33[44m" : " \33[40m");
		else
			if(selected==0) selected++;

		printf("     %sUse IOS 37                       \33[40m\n\n", (selected==1 && (tick_counter & 32)) ? ">\33[44m" : " \33[40m");
		printf("     %sUse IOS 57                       \33[40m\n\n", (selected==2 && (tick_counter & 32)) ? ">\33[44m" : " \33[40m");
		printf("     %sUse IOS 60                       \33[40m\n\n", (selected==3 && (tick_counter & 32)) ? ">\33[44m" : " \33[40m");
		}

	else 
		{
		printf("     %sUse IOS 38 (Recommended)         \33[40m\n\n", (selected==0 && (tick_counter & 32)) ? ">\33[44m" : " \33[40m");
		selected=0;
		printf("\nNote: You can only install cIOS 222 with IOS 38 because other IOS\nsupported in this installer don't works installing channels\n(ES error -1029. Its works fine installing IOS or for NAND access)\n\n");
		}

	switch(selected)
	{
	case 0:
        // IOS 38
		INPUT_TITLEID_L= 38;
		INPUT_VERSION= 3867/*3610*/;
		break;
	case 1:
        // IOS 37
		INPUT_TITLEID_L= 37;
		INPUT_VERSION= 3869 /*3612*/;
		break;

	case 2:
        // IOS 57
		INPUT_TITLEID_L=57;
		INPUT_VERSION=5661 /*5404*/;
		break;
	
	case 3:
        // IOS 60
		INPUT_TITLEID_L=60;
		INPUT_VERSION=6174;
		break;
	/*
	case 3:
        // IOS 36
		INPUT_TITLEID_L= 36;
		INPUT_VERSION= 1042;
		break;
		*/
	}
	
	printf("\n\n     Press A to select or B to Abort\n\n");


	printf("To install the current selection you need the files in:\n    \33[33msd:/ios/%08x%08xv%d\33[37m\n\n", INPUT_TITLEID_H, INPUT_TITLEID_L, INPUT_VERSION);
	printf("Use the NUS Download application if you cannot access to Internet from the Wii and copy the files to the sd:/ios/ folder (no wads)\n\n");


		WPAD_ScanPads();
		pressed = WPAD_ButtonsDown(0);

		if(pressed) {
			if (pressed == WPAD_BUTTON_A) {
				break;
			} 
			if (pressed == WPAD_BUTTON_B) {
				printf("Aborted, exiting...\n");
				return 0;
			} 
			if (pressed == WPAD_BUTTON_UP) {
				selected--;if(selected<0) selected=0;
			} 

			if (pressed == WPAD_BUTTON_DOWN) {
				selected++;if(selected>3) selected=3;
			}
		}
		VIDEO_WaitVSync();
		tick_counter++;
		if(exit_by_reset) {
				printf("Aborted, exiting...\n");
				return 0;
			} 
	}

	
	adjust_patch(INPUT_TITLEID_L);
	
	if (fatInitDefault()) {
		chdir ("sd:/");
	}
	else useSd=0;

	rv = patchmii();

		if(useSd) fatUnmount("sd");

	return rv;
}

int apply_patch(u8 *data, u32 offset, u8 *orig, u32 orig_size, u8 *patch, u32 patch_size)
{
	if (memcmp(&data[offset], orig, orig_size) == 0) {
		memcpy(&data[offset], patch, patch_size);
		return -1;
	} else {
		return 0;
	}
}



int patch_dip(u8 * decrypted_buf)
{
	if (!apply_patch(decrypted_buf, DIP_patch1_pos, DIP_orig1, sizeof(DIP_orig1), DIP_patch1, sizeof(DIP_patch1))) {
		printf("DIP patch 1 failed.\n");
		return 0;
	}

	if (!apply_patch(decrypted_buf, DIP_DVD_enable_orig_pos1, DIP_DVD_enable_orig, sizeof(DIP_DVD_enable_orig), DIP_DVD_enable_patch, sizeof(DIP_DVD_enable_patch))) {
		printf("DIP DVD enable patch 1 failed.\n");
		return 0;
	}

	if (!apply_patch(decrypted_buf, DIP_DVD_enable_orig_pos2, DIP_DVD_enable_orig, sizeof(DIP_DVD_enable_orig), DIP_DVD_enable_patch, sizeof(DIP_DVD_enable_patch))) {
		printf("DIP DVD enable patch 2 failed.\n");
		return 0;
	}


	if(OUTPUT_TITLEID_L!=202)
		{
		/* Replace function handle DI command. */

		if (!apply_patch(decrypted_buf, DIP_handle_di_cmd, DIP_handle_di_cmd_orig, sizeof(DIP_handle_di_cmd_orig),
			patch_handle_di_cmd,len_patch_handle_di_cmd)) {
			printf("DIP A8 patch failed.\n");
			return 0;
			}

		debug_printf("Patched DIP handle cmd.\n");

		// apply patch directly
		memcpy(&decrypted_buf[DIP_handle_di_cmd_reentry], handle_di_cmd_reentry, len_handle_di_cmd_reentry);

		}
	
				
return 1;
}
static int patchmii(void)
{
#if SAVE_DECRYPTED
char name[256];
FILE *fd;
#endif
// ******* WARNING *******
// Obviously, if you're reading this, you're obviously capable of disabling the
// following checks.  If you put any of the following titles into an unusuable state, 
// your Wii will fail to boot:
//
// 1-1 (BOOT2), 1-2 (System Menu), 1-30 (IOS30, currently specified by 1-2's TMD)
// Corrupting other titles (for example, BC or the banners of installed channels)
// may also cause difficulty booting.  Please do not remove these safety checks
// unless you have performed extensive testing and are willing to take on the risk
// of bricking the systems of people to whom you give this code.  -bushing

/*	if ((OUTPUT_TITLEID_H == 1) && (OUTPUT_TITLEID_L == 2)) {
		printf("Sorry, I won't modify the system menu; too dangerous. :(\n");
		while(1);
  	}

	if ((OUTPUT_TITLEID_H == 1) && (OUTPUT_TITLEID_L == 30)) {
		printf("Sorry, I won't modify IOS30; too dangerous. :(\n");
		while(1);
  	}
*/


	printvers();
  
 
	int retval;

	if (ISFS_Initialize() || create_temp_dir()) {
		perror("Failed to create temp dir: ");
		return(1);
	}

  	signed_blob *s_tmd = NULL, *s_tik = NULL, *s_certs = NULL;

  	u8 *temp_tmdbuf = NULL, *temp_tikbuf = NULL;

  	static u8 tmdbuf[MAX_SIGNED_TMD_SIZE*2] ATTRIBUTE_ALIGN(0x20);
  	static u8 tikbuf[STD_SIGNED_TIK_SIZE*2] ATTRIBUTE_ALIGN(0x20);
  
  	u32 tmdsize;
	int update_tmd;
	static char tmdname[32];


	if (useSd) 
		{
		snprintf(buf, 128, "sd:/ios");
		mkdir(buf,S_IREAD | S_IWRITE);
		
		snprintf(buf, 128, "sd:/ios/%08x%08xv%d", INPUT_TITLEID_H, INPUT_TITLEID_L, INPUT_VERSION);
		mkdir(buf,S_IREAD | S_IWRITE);
		}


  	debug_printf("Downloading IOS%d metadata: ..", INPUT_TITLEID_L);
	sleep(2);
	snprintf(tmdname, sizeof(tmdname),"tmd.%d", INPUT_VERSION);
  	retval = get_nus_object(INPUT_TITLEID_H, INPUT_TITLEID_L, INPUT_VERSION, tmdname, &temp_tmdbuf, &tmdsize);
  	if (retval<0) {
		error_debug_printf("get_nus_object(tmd) returned %d, tmdsize = %u", retval, tmdsize);
		return(1);
	}
	if (temp_tmdbuf == NULL) {
		error_debug_printf("Failed to allocate temp buffer for encrypted content, size was %u", tmdsize);
		return(1);
	}
  	memcpy(tmdbuf, temp_tmdbuf, MIN(tmdsize, sizeof(tmdbuf)));
	free(temp_tmdbuf);

	s_tmd = (signed_blob *)tmdbuf;
	if(!IS_VALID_SIGNATURE(s_tmd)) {
    	error_debug_printf("Bad TMD signature!");
		return(1);
  	}

  	debug_printf("\b ..tmd..");

	u32 ticketsize;
	retval = get_nus_object(INPUT_TITLEID_H, INPUT_TITLEID_L, INPUT_VERSION,
						  "cetk", &temp_tikbuf, &ticketsize);
						
	if (retval < 0) error_debug_printf("get_nus_object(cetk) returned %d, ticketsize = %u", retval, ticketsize);
	memcpy(tikbuf, temp_tikbuf, MIN(ticketsize, sizeof(tikbuf)));
  
	s_tik = (signed_blob *)tikbuf;
	if(!IS_VALID_SIGNATURE(s_tik)) {
    	error_debug_printf("Bad tik signature!");
		return(1);
  	}
  
  	free(temp_tikbuf);

	s_certs = (signed_blob *)haxx_certs;
	if(!IS_VALID_SIGNATURE(s_certs)) {
    	error_debug_printf("Bad cert signature!");
		return(1);
  	}

	debug_printf("\b ..ticket..");

	u8 key[16];
	get_title_key(s_tik, key);
	aes_set_key(key);

	tmd *p_tmd;
	tmd_content *p_cr;
	p_tmd = (tmd*)SIGNATURE_PAYLOAD(s_tmd);
	p_cr = TMD_CONTENTS(p_tmd);

	if (p_tmd->title_version != INPUT_VERSION) {
		printf("TMD Version wrong %d != %d.\n", p_tmd->title_version, INPUT_VERSION);
		return 1;
	}
	/* Patch version number. */
	p_tmd->title_version = OUTPUT_VERSION;
        
	print_tmd_summary(p_tmd);

	debug_printf("Downloading contents: \n");
	static char cidstr[32];
	u16 i;
	for (i=0;i<p_tmd->num_contents;i++) {
	   debug_printf("Downloading part %d/%d (%uK): ", i+1, 
					p_tmd->num_contents, p_cr[i].size / 1024);
	   sprintf(cidstr, "%08x", p_cr[i].cid);
   
	   u8 *content_buf, *decrypted_buf;
	   u32 content_size;

	   retval = get_nus_object(INPUT_TITLEID_H, INPUT_TITLEID_L, INPUT_VERSION, cidstr, &content_buf, &content_size);
	   if (retval < 0) {
			error_debug_printf("get_nus_object(%s) failed with error %d, content size = %u", 
					cidstr, retval, content_size);
			return(1);
		}

		if (content_buf == NULL) {
			error_debug_printf("error allocating content buffer, size was %u", content_size);
			return(1);
		}

		if (content_size % 16) {
			error_debug_printf("ERROR: downloaded content[%hu] size %u is not a multiple of 16",
					i, content_size);
			free(content_buf);
			return(1);
		}

   		if (content_size < p_cr[i].size) {
			error_debug_printf("ERROR: only downloaded %u / %llu bytes", content_size, p_cr[i].size);
			free(content_buf);
			return(1);
   		} 

		decrypted_buf = malloc(content_size);
		if (!decrypted_buf) {
			error_debug_printf("ERROR: failed to allocate decrypted_buf (%u bytes)", content_size);
			free(content_buf);
			return(1);
		}

		decrypt_buffer(i, content_buf, decrypted_buf, content_size);

		sha1 hash;
		SHA1(decrypted_buf, p_cr[i].size, hash);

		if (!memcmp(p_cr[i].hash, hash, sizeof hash)) {
                  debug_printf("\b hash OK.\n");
			//display_ios_tags(decrypted_buf, content_size);
		

#if SAVE_DECRYPTED

sprintf(name,"sd:/modulo_%s.elf",cidstr);
				fd = fopen(name, "wb");
				if (fd) {
				fwrite(decrypted_buf, content_size, 1, fd);
				fclose(fd);
				}
#else			

			update_tmd = 0;
			switch (p_cr[i].cid) {
			case 0x00000000:
				break;

			case 0x00000001: /* DIP */
           
			   if(INPUT_TITLEID_L==36 || INPUT_TITLEID_L==38 /*|| INPUT_TITLEID_L==57 old v5404*/|| INPUT_TITLEID_L==60)
					{

					printf("DIP Patch\n");
	
					if(!patch_dip(decrypted_buf)) return 0;
					debug_printf("Patched DIP.\n");
					update_tmd = 1;

					
					}

					break;

			case 0x00000010: /* DIP */
           
				if(INPUT_TITLEID_L==37)
					{

					printf("DIP Patch\n");
			
					if(!patch_dip(decrypted_buf)) return 0;
			
					debug_printf("Patched DIP.\n");
					update_tmd = 1;
					}
				break;

			case 0x000000016: /* DIP */
           
			   if(INPUT_TITLEID_L==57 )
					{

					printf("DIP Patch\n");
					if(!patch_dip(decrypted_buf)) return 0;
			
					debug_printf("Patched DIP.\n");
					update_tmd = 1;
					}

					break;
				
		
			case 0x0000000e: /* FFS, ES, IOSP */
				if(INPUT_TITLEID_L==36 || INPUT_TITLEID_L==60)
					{
					
					printf("Patch ES\n");

					IRQS_patchs((unsigned char * ) decrypted_buf, content_size);
					
					update_tmd = 1;
					}

				break;
        
			
			/*0x00000011 old 38 v3610 */
			case 0x00000014 : /* FFS, ES, IOSP */
			 if(INPUT_TITLEID_L==38)
				{ // 1
				
				printf("Patch ES\n");

				IRQS_patchs((unsigned char * ) decrypted_buf, content_size);
		
				
				update_tmd = 1;
				} // 1

				break;
			// case 0x000000012: old ios57 v5404
			case 0x000000017:/* FFS, ES, IOSP */
				if(INPUT_TITLEID_L==57)
					{

					printf("Patch ES\n");

					IRQS_patchs((unsigned char * ) decrypted_buf, content_size);
					
					update_tmd = 1;
					}

				break;

			// case 0x0000001b: old 37 (v3612)
			case 0x0000001e: 
				if(INPUT_TITLEID_L==37) /* FFS, ES, IOSP */
					{ // 1
					
					printf("Patch ES\n");

					IRQS_patchs((unsigned char * ) decrypted_buf, content_size);


					update_tmd = 1;
					} // 1

				break;

			default:
				break;
			}

			if(update_tmd == 1) {
				debug_printf("Updating TMD.\n");
				SHA1(decrypted_buf, p_cr[i].size, hash);
				memcpy(p_cr[i].hash, hash, sizeof hash);
				if (p_cr[i].type == 0x8001) {
					p_cr[i].type = 1;
				}
				tmd_dirty=1;
			}

			retval = (int) save_nus_object(p_cr[i].cid, decrypted_buf, content_size);
			if (retval < 0) {
				error_debug_printf("save_nus_object(%x) returned error %d", p_cr[i].cid, retval);
				return(1);
			}
#endif   // save_decrypt

		} else {
			error_debug_printf("hash BAD");
			return(1);
		}
   
		free(decrypted_buf);
	   	free(content_buf);
	}

#ifndef SAVE_DECRYPTED
        
		if(add_custom_modules(p_tmd))
                tmd_dirty=1;

	if ((INPUT_TITLEID_H != OUTPUT_TITLEID_H) 
		|| (INPUT_TITLEID_L != OUTPUT_TITLEID_L)) {
		debug_printf("Changing titleid from %08x-%08x to %08x-%08x\n",
			INPUT_TITLEID_H, INPUT_TITLEID_L,
			OUTPUT_TITLEID_H, OUTPUT_TITLEID_L);
		change_ticket_title_id(s_tik, OUTPUT_TITLEID_H, OUTPUT_TITLEID_L);
		change_tmd_title_id(s_tmd, OUTPUT_TITLEID_H, OUTPUT_TITLEID_L);
	} 
	aes_set_key(key);

	if (tmd_dirty) {
    	forge_tmd(s_tmd);
    	tmd_dirty = 0;
  	}

  	if (tik_dirty) {
    	forge_tik(s_tik);
    	tik_dirty = 0;
  	}

  	//debug_printf("Download complete. Installing:\n");

	printf("\33[2J\n\n\33[46m\33[2K\n\33[2K Download completed.\n\33[2K Press button 1 for Install (Take the Risk) or B to Abort\n\33[2K\33[40m\n\n");
	while(1)
		{
		s32 pressed;
		WPAD_ScanPads();
		pressed = WPAD_ButtonsDown(0);

		if(pressed) {
			if (pressed == WPAD_BUTTON_B) {
				printf("Aborted, exiting...\n");
				return 0;
				break;
			} 

			if (pressed == WPAD_BUTTON_1) {
				break;
			} 
		}
	
	VIDEO_WaitVSync();

	if(exit_by_reset) {
			printf("Aborted, exiting...\n");
			return 0;
			}
			
	}

  	retval = install_ticket(s_tik, s_certs, haxx_certs_size);
  	if (retval) {
    	error_debug_printf("install_ticket returned %d", retval);
		return(1);
  	}

  	retval = install(s_tmd, s_certs, haxx_certs_size);
#endif 
  	if (retval) {
    	error_debug_printf("install returned %d", retval);
    	return(1);
  	}

  	debug_printf("Done!\n");

	return(0);
}
