/*
 Copyright (c) 2008 Francisco Muñoz 'Hermes' <www.elotrolado.net>
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are
 permitted provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice, this list of
 conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above copyright notice, this list
 of conditions and the following disclaimer in the documentation and/or other
 materials provided with the distribution.
 - The names of the contributors may not be used to endorse or promote products derived
 from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "oggplayer.h"
#include <gccore.h>

/* OGG control */

#define READ_SAMPLES 4096 // samples that it must read before to send
#define MAX_PCMOUT 4096 // minimum size to read ogg samples
typedef struct
{
	OggVorbis_File vf;
	vorbis_info *vi;
	int current_section;

	// OGG file operation
	int fd;
	int mode;
	int eof;
	int flag;
	int volume;
	int seek_time;

	/* OGG buffer control */
	short pcmout[2][READ_SAMPLES + MAX_PCMOUT * 2]; /* take 4k out of the data segment, not the stack */
	int pcmout_pos;
	int pcm_indx;

} private_data_ogg;

static private_data_ogg private_ogg;

// OGG thread control

#define STACKSIZE		8192

static u8 oggplayer_stack[STACKSIZE];
static lwpq_t oggplayer_queue;
static lwp_t h_oggplayer;
static int ogg_thread_running = 0;

static void ogg_add_callback(int voice)
{
	if (ogg_thread_running <= 0)
	{
		SND_StopVoice(0);
		return;
	}

	if (private_ogg.flag & 128)
		return; // Ogg is paused

	if (private_ogg.pcm_indx >= READ_SAMPLES)
	{
		if (SND_AddVoice(0,
				(void *) private_ogg.pcmout[private_ogg.pcmout_pos],
				private_ogg.pcm_indx << 1) == 0)
		{
			private_ogg.pcmout_pos ^= 1;
			private_ogg.pcm_indx = 0;
			private_ogg.flag = 0;
			LWP_ThreadSignal(oggplayer_queue);
		}
	}
	else
	{
		if (private_ogg.flag & 64)
		{
			private_ogg.flag &= ~64;
			LWP_ThreadSignal(oggplayer_queue);
		}
	}
}

static void * ogg_player_thread(private_data_ogg * priv)
{
	int first_time = 1;

	ogg_thread_running = 0;
	//init
	LWP_InitQueue(&oggplayer_queue);

	priv[0].vi = ov_info(&priv[0].vf, -1);

	SND_Pause(0);

	priv[0].pcm_indx = 0;
	priv[0].pcmout_pos = 0;
	priv[0].eof = 0;
	priv[0].flag = 0;
	priv[0].current_section = 0;

	ogg_thread_running = 1;

	while (!priv[0].eof)
	{
		long ret;
		if (ogg_thread_running <= 0)
			break;

		if (priv[0].flag)
			LWP_ThreadSleep(oggplayer_queue); // wait only when i have samples to send

		if (ogg_thread_running <= 0)
			break;

		if (priv[0].flag == 0) // wait to all samples are sended
		{
			if (SND_TestPointer(0, priv[0].pcmout[priv[0].pcmout_pos])
					&& SND_StatusVoice(0) != SND_UNUSED)
			{
				priv[0].flag |= 64;
				continue;
			}
			if (priv[0].pcm_indx < READ_SAMPLES)
			{
				priv[0].flag = 3;

				if (priv[0].seek_time >= 0)
				{
					ov_time_seek(&priv[0].vf, priv[0].seek_time);
					priv[0].seek_time = -1;
				}

				ret
						= ov_read(
								&priv[0].vf,
								(void *) &priv[0].pcmout[priv[0].pcmout_pos][priv[0].pcm_indx],
								MAX_PCMOUT,/*0,2,1,*/&priv[0].current_section);
				priv[0].flag &= 192;
				if (ret == 0)
				{
					/* EOF */
					if (priv[0].mode & 1)
						ov_time_seek(&priv[0].vf, 0); // repeat
					else
						priv[0].eof = 1; // stops
					//
				}
				else if (ret < 0)
				{
					/* error in the stream.  Not a problem, just reporting it in
					 case we (the app) cares.  In this case, we don't. */
					if (ret != OV_HOLE)
					{
						if (priv[0].mode & 1)
							ov_time_seek(&priv[0].vf, 0); // repeat
						else
							priv[0].eof = 1; // stops
					}
				}
				else
				{
					/* we don't bother dealing with sample rate changes, etc, but
					 you'll have to*/
					priv[0].pcm_indx += ret >> 1; //get 16 bits samples
				}
			}
			else
				priv[0].flag = 1;
		}

		if (priv[0].flag == 1)
		{
			if (SND_StatusVoice(0) == SND_UNUSED || first_time)
			{
				first_time = 0;
				if (priv[0].vi->channels == 2)
				{
					SND_SetVoice(0, VOICE_STEREO_16BIT, priv[0].vi->rate, 0,
							(void *) priv[0].pcmout[priv[0].pcmout_pos],
							priv[0].pcm_indx << 1, priv[0].volume,
							priv[0].volume, ogg_add_callback);
					priv[0].pcmout_pos ^= 1;
					priv[0].pcm_indx = 0;
					priv[0].flag = 0;
				}
				else
				{
					SND_SetVoice(0, VOICE_MONO_16BIT, priv[0].vi->rate, 0,
							(void *) priv[0].pcmout[priv[0].pcmout_pos],
							priv[0].pcm_indx << 1, priv[0].volume,
							priv[0].volume, ogg_add_callback);
					priv[0].pcmout_pos ^= 1;
					priv[0].pcm_indx = 0;
					priv[0].flag = 0;
				}
			}
			else
			{
				// if(priv[0].pcm_indx==0) priv[0].flag=0; // all samples sended
			}

		}

	}
	ov_clear(&priv[0].vf);
	priv[0].fd = -1;
	priv[0].pcm_indx = 0;
	ogg_thread_running = 0;

	return 0;
}

void StopOgg()
{
	SND_StopVoice(0);
	if (ogg_thread_running > 0)
	{
		ogg_thread_running = -2;
		LWP_ThreadSignal(oggplayer_queue);
		LWP_JoinThread(h_oggplayer, NULL);

		while (((volatile int) ogg_thread_running) != 0)
		{
			;;;
		}
	}
}

int PlayOgg(int fd, int time_pos, int mode)
{
	StopOgg();

	ogg_thread_running = 0;

	private_ogg.fd = fd;
	private_ogg.mode = mode;
	private_ogg.eof = 0;
	private_ogg.volume = 127;
	private_ogg.flag = 0;
	private_ogg.seek_time = -1;

	if (time_pos > 0)
		private_ogg.seek_time = time_pos;

	if (fd < 0)
	{
		private_ogg.fd = -1;
		return -1;
	}
	if (ov_open((void *) &private_ogg.fd, &private_ogg.vf, NULL, 0) < 0)
	{
		mem_close(private_ogg.fd); // mem_close() can too close files from devices
		private_ogg.fd = -1;
		ogg_thread_running = -1;
		return -1;
	}

	if (LWP_CreateThread(&h_oggplayer, (void *) ogg_player_thread,
			&private_ogg, oggplayer_stack, STACKSIZE, 80) == -1)
	{
		ogg_thread_running = -1;
		ov_clear(&private_ogg.vf);
		private_ogg.fd = -1;
		return -1;
	}
	LWP_ThreadSignal(oggplayer_queue);
	while (((volatile int) ogg_thread_running) == 0)
	{
		;;;
	}
	return 0;
}

void PauseOgg(int pause)
{
	if (pause)
	{
		private_ogg.flag |= 128;
	}
	else
	{
		if (private_ogg.flag & 128)
		{
			private_ogg.flag |= 64;
			private_ogg.flag &= ~128;
			if (ogg_thread_running > 0)
			{
				LWP_ThreadSignal(oggplayer_queue);
				//   while(((volatile int )private_ogg.flag)!=1 && ((volatile int )ogg_thread_running)>0) {;;;}
			}
		}

	}
}

int StatusOgg()
{
	if (ogg_thread_running <= 0)
		return -1; // Error

	if (private_ogg.eof)
		return 255; // EOF

	if (private_ogg.flag & 128)
		return 2; // paused
	return 1; // running
}

void SetVolumeOgg(int volume)
{
	private_ogg.volume = volume;

	SND_ChangeVolumeVoice(0, volume, volume);
}

s32 GetTimeOgg()
{
	int ret;
	if (ogg_thread_running <= 0)
		return 0;
	if (private_ogg.fd < 0)
		return 0;
	ret = ((s32) ov_time_tell(&private_ogg.vf));
	if (ret < 0)
		ret = 0;

	return ret;
}

void SetTimeOgg(s32 time_pos)
{
	if (time_pos >= 0)
		private_ogg.seek_time = time_pos;
}
