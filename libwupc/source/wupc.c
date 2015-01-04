/****************************************************************************
 * Copyright (C) 2014 FIX94
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

/* WiiU Pro Controller Documentation from TeHaxor69 */

#include <gccore.h>
#include <ogc/machine/processor.h>
#include <wiiuse/wiiuse.h>
#include <stdio.h>
#include <string.h>
#include "wupc_structs.h"
#include "wupc/wupc.h"

extern __typeof(wiiuse_register) __real_wiiuse_register;

static vu32 __WUPC_ChannelsUsed = 0;

static conf_pads __WUPC_Devices;
static struct WUPCStat *__WUPC_Connected[4];
static struct WUPCStat __WUPC_Status[CONF_PAD_MAX_REGISTERED];
static struct WUPCData __WUPC_PadData[4];
static struct WUPCButtons __WUPC_PadButtons[4];

static u32 __WUPC_Inited = 0;

static const u8 __WUPC_LEDState[] = { 0x10, 0x20, 0x40, 0x80 };

#define CHAN_MAX 4

#define TRANSFER_CALIBRATE 0
#define TRANSFER_DONE 1

static void __WUPC_SetLED(struct bte_pcb *sock, u32 state)
{
	u8 buf[2];
	buf[0] = 0x11;
	buf[1] = __WUPC_LEDState[state];
	bte_senddata(sock,buf,2);
}
static s32 __WUPC_HandleData(void *arg,void *buffer,u16 len)
{
	struct WUPCStat *stat = (struct WUPCStat*)arg;
	u32 chan = stat->channel;

	if(*(u8*)buffer == 0x3D && len == 22)
	{
		if(stat->transferstate == TRANSFER_CALIBRATE)
		{
			stat->xAxisLmid = bswap16(*(u16*)(((u8*)buffer)+1));
			stat->xAxisRmid = bswap16(*(u16*)(((u8*)buffer)+3));
			stat->yAxisLmid = bswap16(*(u16*)(((u8*)buffer)+5));
			stat->yAxisRmid = bswap16(*(u16*)(((u8*)buffer)+7));
			stat->transferstate = TRANSFER_DONE;
		}
		__WUPC_PadData[chan].xAxisL = bswap16(*(u16*)(((u8*)buffer)+1)) - stat->xAxisLmid;
		__WUPC_PadData[chan].xAxisR = bswap16(*(u16*)(((u8*)buffer)+3)) - stat->xAxisRmid;
		__WUPC_PadData[chan].yAxisL = bswap16(*(u16*)(((u8*)buffer)+5)) - stat->yAxisLmid;
		__WUPC_PadData[chan].yAxisR = bswap16(*(u16*)(((u8*)buffer)+7)) - stat->yAxisRmid;
		__WUPC_PadData[chan].button = ~(*(u16*)(((u8*)buffer)+9)) << 16;
		u8 extradata = ~(*(((u8*)buffer)+11));
		__WUPC_PadData[chan].battery = (extradata >> 4) & 0x7;
		__WUPC_PadData[chan].extra = extradata & 0xF;
	}
	return ERR_OK;
}

static s32 __WUPC_HandleConnect(void *arg,struct bte_pcb *pcb,u8 err)
{
	struct WUPCStat *stat = (struct WUPCStat*)arg;

	if(__WUPC_ChannelsUsed >= CHAN_MAX)
	{
		bte_disconnect(pcb);
		return err;
	}

	stat->channel = __WUPC_ChannelsUsed;
	stat->rumble = 0;

	__WUPC_SetLED(pcb, __WUPC_ChannelsUsed);

	u8 buf[3];
	buf[0] = 0x12;
	buf[1] = 0x00;
	buf[2] = 0x3D;
	bte_senddata(pcb,buf,3);
	stat->transferstate = TRANSFER_CALIBRATE;

	__WUPC_Connected[__WUPC_ChannelsUsed] = stat;
	__WUPC_ChannelsUsed++;
	return err;
}

static s32 __WUPC_HandleDisconnect(void *arg,struct bte_pcb *pcb __attribute__((unused)),u8 err)
{
	struct WUPCStat *stat = (struct WUPCStat*)arg;
	if(__WUPC_ChannelsUsed) __WUPC_ChannelsUsed--;

	u32 i;
	for(i = stat->channel; i < 3; ++i)
		__WUPC_Connected[i] = __WUPC_Connected[i+1];
	__WUPC_Connected[3] = NULL;

	for(i = 0; i < CONF_PAD_MAX_REGISTERED; ++i)
	{
		if(__WUPC_Status[i].channel > stat->channel && __WUPC_Status[i].channel != CHAN_MAX)
		{
			__WUPC_Status[i].channel--;
			__WUPC_SetLED(__WUPC_Status[i].sock, __WUPC_Status[i].channel);
		}
	}
	stat->channel = CHAN_MAX;
	return err;
}

int __WUPC_RegisterPad(struct WUPCStat *stat, struct bd_addr *_bdaddr)
{
	stat->channel = CHAN_MAX;
	stat->bdaddr = *_bdaddr;

	if(stat->sock == NULL)
	{
		stat->sock = bte_new();
		if(stat->sock == NULL)
			return ERR_OK;
	}

	bte_arg(stat->sock, stat);
	bte_received(stat->sock, __WUPC_HandleData);
	bte_disconnected(stat->sock, __WUPC_HandleDisconnect);

	bte_registerdeviceasync(stat->sock, _bdaddr, __WUPC_HandleConnect);

	return ERR_OK;
}

int __wrap_wiiuse_register(struct wiimote_listen_t *wml, struct bd_addr *bdaddr, struct wiimote_t *(*assign_cb)(struct bd_addr *bdaddr))
{
	if(__WUPC_Inited)
	{
		u8 got_addr[] = { bdaddr->addr[5], bdaddr->addr[4], bdaddr->addr[3], bdaddr->addr[2], bdaddr->addr[1], bdaddr->addr[0] };
		u32 i;
		for(i = 0; i < __WUPC_Devices.num_registered; ++i)
		{
			if(strstr(__WUPC_Devices.registered[i].name, "-UC") != NULL)
			{
				u8 *cur_bdaddr = __WUPC_Devices.registered[i].bdaddr;
				if(memcmp(cur_bdaddr, got_addr, 6) == 0)
					return __WUPC_RegisterPad(&__WUPC_Status[i], bdaddr);
			}
		}
	}
	return __real_wiiuse_register(wml,bdaddr,assign_cb);
}

void WUPC_Init()
{
	if(__WUPC_Inited == 1)
		return;
	if(CONF_GetPadDevices(&__WUPC_Devices) < 0)
		return;

	u32 i;
	for(i = 0; i < CHAN_MAX; ++i)
		__WUPC_Connected[i] = NULL;

	for(i = 0; i < CONF_PAD_MAX_REGISTERED; ++i)
		__WUPC_Status[i].channel = CHAN_MAX;

	__WUPC_ChannelsUsed = 0;
	__WUPC_Inited = 1;
}

void WUPC_Disconnect(u8 chan)
{
	if(chan >= CHAN_MAX || __WUPC_Connected[chan] == NULL) return;
	bte_disconnect(__WUPC_Connected[chan]->sock);
}

void WUPC_Shutdown()
{
	if(__WUPC_Inited == 0)
		return;

	__WUPC_Inited = 0;

	u32 i;
	for(i = 0; i < CONF_PAD_MAX_REGISTERED; ++i)
	{
		if(__WUPC_Status[i].channel != CHAN_MAX)
			bte_disconnect(__WUPC_Status[i].sock);
	}
}

struct WUPCData *WUPC_Data(u8 chan)
{
	if(chan >= CHAN_MAX || __WUPC_Connected[chan] == NULL) return NULL;
	return &__WUPC_PadData[chan];
}

void WUPC_Rumble(u8 chan, bool rumble)
{
	if(chan >= CHAN_MAX || __WUPC_Connected[chan] == NULL) return;

	u8 buf[2];
	buf[0] = 0x11;
	buf[1] = __WUPC_LEDState[chan] | rumble;
	bte_senddata(__WUPC_Connected[chan]->sock,buf,2);
}

u32 WUPC_UpdateButtonStats()
{
	u32 newstate, oldstate;
	u32 i, ret = 0;
	for(i = 0; i < CHAN_MAX; ++i)
	{
		if(__WUPC_Connected[i] == NULL)
			return ret;
		newstate = __WUPC_PadData[i].button | (__WUPC_PadData[i].extra & WUPC_EXTRA_BUTTON_RSTICK) | (__WUPC_PadData[i].extra & WUPC_EXTRA_BUTTON_LSTICK);
		oldstate = __WUPC_PadButtons[i].state;
		__WUPC_PadButtons[i].state = newstate;
		__WUPC_PadButtons[i].up = oldstate & ~newstate;
		__WUPC_PadButtons[i].down = newstate & (newstate ^ oldstate);
		ret |= (1<<i);
	}
	return ret;
}

u32 WUPC_ButtonsUp(u8 chan)
{
	if(chan >= CHAN_MAX || __WUPC_Connected[chan] == NULL) return 0;
	return __WUPC_PadButtons[chan].up;
}
u32 WUPC_ButtonsDown(u8 chan)
{
	if(chan >= CHAN_MAX || __WUPC_Connected[chan] == NULL) return 0;
	return __WUPC_PadButtons[chan].down;
}
u32 WUPC_ButtonsHeld(u8 chan)
{
	if(chan >= CHAN_MAX || __WUPC_Connected[chan] == NULL) return 0;
	return __WUPC_PadButtons[chan].state;
}
s16 WUPC_lStickX(u8 chan)
{
	if(chan >= CHAN_MAX || __WUPC_Connected[chan] == NULL) return 0;
	return __WUPC_PadData[chan].xAxisL;
}
s16 WUPC_lStickY(u8 chan)
{
	if(chan >= CHAN_MAX || __WUPC_Connected[chan] == NULL) return 0;
	return __WUPC_PadData[chan].yAxisL;
}
s16 WUPC_rStickX(u8 chan)
{
	if(chan >= CHAN_MAX || __WUPC_Connected[chan] == NULL) return 0;
	return __WUPC_PadData[chan].xAxisR;
}
s16 WUPC_rStickY(u8 chan)
{
	if(chan >= CHAN_MAX || __WUPC_Connected[chan] == NULL) return 0;
	return __WUPC_PadData[chan].yAxisR;
}
u8 WUPC_extra(u8 chan)
{
	if(chan >= CHAN_MAX || __WUPC_Connected[chan] == NULL) return 0;
	return __WUPC_PadData[chan].extra;
}
