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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <gcutil.h>
#include <ogc/lwp_queue.h>
#include <gccore.h>	
 
#include "dvd_broadway.h"
 
#define DI_CMDCTX_CNT				4
 
#define DVD_DISKIDSIZE				0x20
#define DVD_DRVINFSIZE				0x20
 
#define IOCTL_DI_INQUIRY			0x12
#define IOCTL_DI_READID				0x70
#define IOCTL_DI_READ				0x71
#define IOCTL_DI_WAITCVRCLOSE		0x79
#define IOCTL_DI_COVER				0x7A
#define IOCTL_DI_RESETNOTIFY		0x7E
#define IOCTL_DI_RESET				0x8A
#define IOCTL_DI_OPENPART			0x8B
#define IOCTL_DI_CLOSEPART			0x8C
#define IOCTL_DI_UNENCREAD			0x8D
#define IOCTL_DI_ENABLE_DVD			0x8E
#define IOCTL_DI_SEEK				0xAB
#define IOCTL_DI_READ_DVDVIDEO		0xD0
#define IOCTL_DI_STOPLASER			0xD2
#define IOCTL_DI_OFFSET				0xD9
#define IOCTL_DI_REQERROR			0xE0
#define IOCTL_DI_STOPMOTOR			0xE3
#define IOCTL_DI_SETOFFBASE			0xF0
#define IOCTL_DI_GETOFFBASE			0xF1
#define IOCTL_DI_SETCRYPTMODE		0xF2
#define IOCTL_DI_GETCRYPTMODE		0xF3
#define IOCTL_DI_SETDVDROMMODE		0xF4
#define IOCTL_DI_GETDVDROMMODE		0xF5
 
#define _SHIFTL(v, s, w)	\
    ((u32) (((u32)(v) & ((0x01 << (w)) - 1)) << (s)))
#define _SHIFTR(v, s, w)	\
    ((u32)(((u32)(v) >> (s)) & ((0x01 << (w)) - 1)))
 
struct dicommand
{
	u32 diReg[8];
};
 
struct dicontext
{
	lwp_node node;
	dvdcallbacklow cb;
	struct dicommand *cmd;
};
 
static s32 __dvd_fd = -1;
static u32 __dvd_spinupval = 1;
static lwp_queue __di_contextq;
static u32 __dvd_readlength = 0;
static u32 __dvd_cbinprogress = 0;
static u32 __dvd_reqinprogress = 0;
static u32 __dvd_lowinitcalled = 0;
static struct dicommand *__di_commands = NULL;
static struct dicontext __di_contexts[DI_CMDCTX_CNT];
static u32 __di_regbuffer[0x08] ATTRIBUTE_ALIGN(32);
static u32 __di_regvalcache[0x08] ATTRIBUTE_ALIGN(32);
static u32 __di_lastticketerror[0x08] ATTRIBUTE_ALIGN(32);
static ioctlv __di_iovector[0x08] ATTRIBUTE_ALIGN(32);
static char __di_fs[] ATTRIBUTE_ALIGN(32) = "/dev/di";
 
extern u32 __IPC_ClntInit();
 
static __inline__ lwp_node* __lwp_queue_head(lwp_queue *queue)
{
	return (lwp_node*)queue;
}
 
static __inline__ lwp_node* __lwp_queue_tail(lwp_queue *queue)
{
	return (lwp_node*)&queue->perm_null;
}
 
 
static __inline__ void __lwp_queue_init_empty(lwp_queue *queue)
{
	queue->first = __lwp_queue_tail(queue);
	queue->perm_null = NULL;
	queue->last = __lwp_queue_head(queue);
}
 
static struct dicontext* __dvd_getcontext(dvdcallbacklow cb)
{
	struct dicontext *ctx;
 
	ctx = (struct dicontext*)__lwp_queue_get(&__di_contextq);
	if(ctx!=NULL) ctx->cb = cb;
 
	return ctx;
}
 
static s32 __dvd_iostransactionCB(s32 result,void *usrdata)
{
	struct dicontext *ctx = (struct dicontext*)usrdata;
 
	__dvd_reqinprogress = 0;
 
	if(ctx->cb!=NULL) {
		__dvd_cbinprogress = 1;
		if(result!=0) __dvd_readlength = 0;
		ctx->cb(result);
		__dvd_cbinprogress = 0;
	}
	__lwp_queue_append(&__di_contextq,&ctx->node);
 
	return 0;
}
 
static s32 __dvd_ioscoverregisterCB(s32 result,void *usrdata)
{
	struct dicontext *ctx = (struct dicontext*)usrdata;
 
	__dvd_reqinprogress = 0;
	__di_regvalcache[1] = __di_regbuffer[0];
 
	if(ctx->cb!=NULL) {
		__dvd_cbinprogress = 1;
		ctx->cb(result);
		__dvd_cbinprogress = 0;
	}
	__lwp_queue_append(&__di_contextq,&ctx->node);
 
	return 0;
}
 
static s32 __dvd_ioscovercloseCB(s32 result,void *usrdata)
{
	struct dicontext *ctx = (struct dicontext*)usrdata;
 
	__dvd_reqinprogress = 0;
 
	if(ctx->cb!=NULL) {
		__dvd_cbinprogress = 1;
		ctx->cb(result);
		__dvd_cbinprogress = 0;
	}
	__lwp_queue_append(&__di_contextq,&ctx->node);
 
	return 0;
}
 
s32 bwDVD_LowInit()
{
	s32 i,ret = 0;
	u32 ipclo,ipchi;
	lwp_queue inactives;
	struct dicontext *ctx;
 
	if(__dvd_lowinitcalled==0) {
		ret = __IPC_ClntInit();
		if(ret<0) return ret;
 
		ipclo = (((u32)IPC_GetBufferLo()+0x1f)&~0x1f);
		ipchi = (u32)IPC_GetBufferHi();
		if(ipchi>=(ipclo+(sizeof(struct dicommand)*DI_CMDCTX_CNT))) {
			__di_commands = (struct dicommand*)ipclo;
			IPC_SetBufferLo((void*)(ipclo+(sizeof(struct dicommand)*DI_CMDCTX_CNT)));
 
			memset(__di_commands,0,(sizeof(struct dicommand)*DI_CMDCTX_CNT));
 
			i = 0;
			__lwp_queue_init_empty(&__di_contextq);
			__lwp_queue_initialize(&inactives,__di_contexts,DI_CMDCTX_CNT,sizeof(struct dicontext));
			while((ctx=(struct dicontext*)__lwp_queue_get(&inactives))!=NULL) {
				ctx->cmd = &__di_commands[i];
				ctx->cb = NULL;
				__lwp_queue_append(&__di_contextq,&ctx->node);
 
				i++;
			}
		}
 
		ret = IOS_Open(__di_fs,0);
		if(ret<0) return ret;
 
		__dvd_fd = ret;
//		__dvd_lowinitcalled = 1;
 
	//	printf("DVD_LowInit(%d)\n",ret);
	}
	return 0;
}
 
s32 bwDVD_LowInquiry(dvddrvinfo *info,dvdcallbacklow cb)
{
	s32 ret;
	struct dicontext *ctx;
	struct dicommand *cmd;
 
	__dvd_reqinprogress = 1;
 
	ctx = __dvd_getcontext(cb);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_INQUIRY<<24);
	ret = IOS_IoctlAsync(__dvd_fd,IOCTL_DI_INQUIRY,cmd->diReg,sizeof(struct dicommand),info,DVD_DRVINFSIZE,__dvd_iostransactionCB,ctx);
 
	return ret;
}
 
s32 bwDVD_LowReadID(dvddiskid *diskID,dvdcallbacklow cb)
{
	s32 ret = 0;
	struct dicontext *ctx;
	struct dicommand *cmd;
 
//	printf("DVD_LowReadID()\n");
 
	__dvd_reqinprogress = 1;
 
	ctx = __dvd_getcontext(cb);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_READID<<24);
	ret = IOS_IoctlAsync(__dvd_fd,IOCTL_DI_READID,cmd->diReg,sizeof(struct dicommand),diskID,DVD_DISKIDSIZE,__dvd_iostransactionCB,ctx);
 
//	printf("DVD_LowReadID(%d)\n",ret);
	return ret;
}

 
s32 bwDVD_LowRead(void *buf,u32 len,u32 offset,dvdcallbacklow cb)
{
	s32 ret;
	struct dicontext *ctx;
	struct dicommand *cmd;
 
	if(buf==NULL || ((u32)buf%32)!=0) return -1;
 
	__dvd_reqinprogress = 1;
	__dvd_readlength = len;
 
	ctx = __dvd_getcontext(cb);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_READ<<24);
	cmd->diReg[1] = len;
	cmd->diReg[2] = offset;
	ret = IOS_IoctlAsync(__dvd_fd,IOCTL_DI_READ,cmd->diReg,sizeof(struct dicommand),buf,len,__dvd_iostransactionCB,ctx);
 
	return ret;
}

// never got this function working, probably removed from wii
s32 bwDVD_LowReadVideo(void *buf,u32 len,u32 offset,dvdcallbacklow cb)
{
	s32 ret;
	struct dicontext *ctx;
	struct dicommand *cmd;
 
	__dvd_reqinprogress = 1;
	__dvd_readlength = len;
 
	ctx = __dvd_getcontext(cb);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_READ_DVDVIDEO<<24);
	cmd->diReg[1] = len;
	cmd->diReg[2] = offset;
	ret = IOS_IoctlAsync(__dvd_fd,IOCTL_DI_READ_DVDVIDEO,cmd->diReg,sizeof(struct dicommand),buf,len,__dvd_iostransactionCB,ctx);
 
	return ret;
}

 

s32 bwDVD_LowStopLaser(dvdcallbacklow cb)
{
	s32 ret;
	struct dicontext *ctx;
	struct dicommand *cmd;
 
	__dvd_reqinprogress = 1;
 
	ctx = __dvd_getcontext(cb);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_STOPLASER<<24);
	ret = IOS_IoctlAsync(__dvd_fd,IOCTL_DI_STOPLASER,cmd->diReg,sizeof(struct dicommand),__di_regvalcache,0x20,__dvd_iostransactionCB,ctx);
 
	return ret;
}

// never got this function working, probably removed from wii
s32 bwDVD_EnableVideo(dvdcallbacklow cb)
{
	s32 ret;
	struct dicontext *ctx;
	struct dicommand *cmd;
 
	__dvd_reqinprogress = 1;
 
	ctx = __dvd_getcontext(cb);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_ENABLE_DVD<<24);
	ret = IOS_IoctlAsync(__dvd_fd,IOCTL_DI_ENABLE_DVD,cmd->diReg,sizeof(struct dicommand),__di_regvalcache,0x20,__dvd_iostransactionCB,ctx);
 
	return ret;
}
 
s32 bwDVD_LowSeek(u32 offset,dvdcallbacklow cb)
{
	s32 ret;
	struct dicontext *ctx;
	struct dicommand *cmd;
 
	__dvd_reqinprogress = 1;
 
	ctx = __dvd_getcontext(cb);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_SEEK<<24);
	cmd->diReg[1] = offset;
	ret = IOS_IoctlAsync(__dvd_fd,IOCTL_DI_SEEK,cmd->diReg,sizeof(struct dicommand),NULL,0,__dvd_iostransactionCB,ctx);
 
	return ret;
}
 
s32 bwDVD_LowOffset(u64 offset,dvdcallbacklow cb)
{
	s32 ret;
	u32 *off = (u32*)(void*)(&offset);
	struct dicontext *ctx;
	struct dicommand *cmd;
 
	__dvd_reqinprogress = 1;
 
	ctx = __dvd_getcontext(cb);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_OFFSET<<24);
	cmd->diReg[1] = 0;
	if(off[0]) cmd->diReg[1] = 1;
	cmd->diReg[2] = off[1];
	ret = IOS_IoctlAsync(__dvd_fd,IOCTL_DI_OFFSET,cmd->diReg,sizeof(struct dicommand),__di_regvalcache,0x20,__dvd_iostransactionCB,ctx);
 
	return ret;
}
 
s32 bwDVD_LowPrepareCoverRegister(dvdcallbacklow cb)
{
	s32 ret;
	struct dicontext *ctx;
	struct dicommand *cmd;
 
	__dvd_reqinprogress = 1;
 
	ctx = __dvd_getcontext(cb);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_COVER<<24);
	ret = IOS_IoctlAsync(__dvd_fd,IOCTL_DI_COVER,cmd->diReg,sizeof(struct dicommand),__di_regbuffer,0x20,__dvd_ioscoverregisterCB,ctx);
 
	return ret;
}
 
s32 bwDVD_LowOpenPartition(u32 offset,void *eticket,u32 certin_len,void *certificate_in,void *certificate_out,dvdcallbacklow cb)
{
	s32 ret;
	struct dicontext *ctx;
	struct dicommand *cmd;
 
	if(eticket!=NULL && ((u32)eticket%32)!=0) return -1;
	if(certificate_in!=NULL && ((u32)certificate_in%32)!=0) return -1;
	if(certificate_out!=NULL && ((u32)certificate_out%32)!=0) return -1;
 
	__dvd_reqinprogress = 1;
 
	ctx = __dvd_getcontext(cb);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_OPENPART<<24);
	cmd->diReg[1] = offset;
 
	__di_iovector[0].data = cmd;
	__di_iovector[0].len = sizeof(struct dicommand);
 
	__di_iovector[1].data = eticket;
	if(eticket==NULL) __di_iovector[1].len = 0;
	else __di_iovector[1].len = 676;
 
	__di_iovector[2].data = certificate_in;
	if(certificate_in==NULL) __di_iovector[2].len = 0;
	else __di_iovector[2].len = certin_len;
 
	__di_iovector[3].data = certificate_out;
	__di_iovector[3].len = 18916;
	__di_iovector[4].data = __di_lastticketerror;
	__di_iovector[4].len = 0x20;
	ret = IOS_IoctlvAsync(__dvd_fd,IOCTL_DI_OPENPART,3,2,__di_iovector,__dvd_iostransactionCB,ctx);
 
	return ret;
}
 
s32 bwDVD_LowClosePartition(dvdcallbacklow cb)
{
	s32 ret;
	struct dicontext *ctx;
	struct dicommand *cmd;
 
	__dvd_reqinprogress = 1;
 
	ctx = __dvd_getcontext(cb);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_CLOSEPART<<24);
	ret = IOS_IoctlAsync(__dvd_fd,IOCTL_DI_CLOSEPART,cmd->diReg,sizeof(struct dicommand),NULL,0,__dvd_iostransactionCB,ctx);
 
	return ret;
}
 
s32 bwDVD_LowUnencryptedRead(void *buf,u32 len,u32 offset,dvdcallbacklow cb)
{
	s32 ret;
	struct dicontext *ctx;
	struct dicommand *cmd;
 
	__dvd_reqinprogress = 1;
	__dvd_readlength = len;
 
	ctx = __dvd_getcontext(cb);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_UNENCREAD<<24);
	cmd->diReg[1] = len;
	cmd->diReg[2] = offset;
	ret = IOS_IoctlAsync(__dvd_fd,IOCTL_DI_UNENCREAD,cmd->diReg,sizeof(struct dicommand),buf,len,__dvd_iostransactionCB,ctx);
 
	return ret;
}
 
s32 bwDVD_LowWaitCoverClose(dvdcallbacklow cb)
{
	s32 ret;
	struct dicontext *ctx;
	struct dicommand *cmd;
 
	__dvd_reqinprogress = 1;
 
	ctx = __dvd_getcontext(cb);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_WAITCVRCLOSE<<24);
	ret = IOS_IoctlAsync(__dvd_fd,IOCTL_DI_WAITCVRCLOSE,cmd->diReg,sizeof(struct dicommand),NULL,0,__dvd_ioscovercloseCB,ctx);
 
	return ret;
}
 
s32 bwDVD_LowResetNotify()
{
	s32 ret;
	struct dicontext *ctx;
	struct dicommand *cmd;
 
	if(__dvd_cbinprogress==1) return -1;
 
	ctx = __dvd_getcontext(NULL);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_RESETNOTIFY<<24);
	ret = IOS_Ioctl(__dvd_fd,IOCTL_DI_RESETNOTIFY,cmd->diReg,sizeof(struct dicommand),NULL,0);
 
	return ret;
}
 
s32 bwDVD_LowReset(dvdcallbacklow cb)
{
	s32 ret;
	struct dicontext *ctx;
	struct dicommand *cmd;
 
//	printf("DVD_LowReset()\n");
	__dvd_reqinprogress = 1;
 
	ctx = __dvd_getcontext(cb);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_RESET<<24);
	cmd->diReg[1] = __dvd_spinupval;
	ret = IOS_IoctlAsync(__dvd_fd,IOCTL_DI_RESET,cmd->diReg,sizeof(struct dicommand),NULL,0,__dvd_iostransactionCB,ctx);
 
//	printf("DVD_LowReset(%d)\n",ret);
	return ret;
}
 
s32 bwDVD_LowStopMotor(u8 stop1,u8 stop2,dvdcallbacklow cb)
{
	s32 ret;
	struct dicontext *ctx;
	struct dicommand *cmd;
 
	__dvd_reqinprogress = 1;
 
	ctx = __dvd_getcontext(cb);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_STOPMOTOR<<24);
	cmd->diReg[1] = (stop1<<24);
	cmd->diReg[2] = (stop2<<24);
	ret = IOS_IoctlAsync(__dvd_fd,IOCTL_DI_STOPMOTOR,cmd->diReg,sizeof(struct dicommand),__di_regvalcache,0x20,__dvd_iostransactionCB,ctx);
 
	return ret;
 
}
 
s32 bwDVD_LowRequestError(dvdcallbacklow cb)
{
	s32 ret;
	struct dicontext *ctx;
	struct dicommand *cmd;
 
	__dvd_reqinprogress = 1;
 
	ctx = __dvd_getcontext(cb);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_REQERROR<<24);
	ret = IOS_IoctlAsync(__dvd_fd,IOCTL_DI_REQERROR,cmd->diReg,sizeof(struct dicommand),__di_regvalcache,0x20,__dvd_iostransactionCB,ctx);
 
	return ret;
}


s32 bwDVD_SetDecryption(s32 mode, dvdcallbacklow cb)
{
	s32 ret;
	struct dicontext *ctx;
	struct dicommand *cmd;
 
	__dvd_reqinprogress = 1;
 
	ctx = __dvd_getcontext(cb);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_SETCRYPTMODE<<24);
	cmd->diReg[1] = mode;
	ret = IOS_IoctlAsync(__dvd_fd,IOCTL_DI_SETCRYPTMODE,cmd->diReg,sizeof(struct dicommand),__di_regvalcache,0x20,__dvd_iostransactionCB,ctx);
 
	return ret;
 
}

s32 bwDVD_SetOffset(u32 offset, dvdcallbacklow cb)
{
	s32 ret;
	struct dicontext *ctx;
	struct dicommand *cmd;
 
	__dvd_reqinprogress = 1;
 
	ctx = __dvd_getcontext(cb);
	if(ctx==NULL) return IPC_ENOMEM;
 
	cmd = ctx->cmd;
	cmd->diReg[0] = (IOCTL_DI_SETOFFBASE<<24);
	cmd->diReg[1] = offset;
	ret = IOS_IoctlAsync(__dvd_fd,IOCTL_DI_SETOFFBASE,cmd->diReg,sizeof(struct dicommand),__di_regvalcache,0x20,__dvd_iostransactionCB,ctx);
 
	return ret;
 
}
