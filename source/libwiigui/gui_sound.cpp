/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_sound.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

/**
 * Constructor for the GuiSound class.
 */
GuiSound::GuiSound(const u8 * snd, s32 len, int t)
{
	sound = snd;
	length = len;
	type = t;
	voice = -1;
	volume = 100;
	loop = false;
}

GuiSound::GuiSound(const u8 * snd, s32 len, int t, int v)
{
	sound = snd;
	length = len;
	type = t;
	voice = -1;
	volume = v;
	loop = false;
}

/**
 * Destructor for the GuiSound class.
 */
GuiSound::~GuiSound()
{
	if(type == SOUND_OGG)
		StopOgg();
}

int GuiSound::PlayOggFile(char * path)
{   /*
    u32 filesize = 0;
    char * bufferogg = NULL;
    size_t resultogg;

    FILE * pFile;
    pFile = fopen (path, "rb");

    // get file size:
    fseek (pFile , 0 , SEEK_END);
    filesize = ftell (pFile);
    rewind (pFile);

    // allocate memory to contain the whole file:
    bufferogg = (char*) malloc (sizeof(char)*filesize);
    if (bufferogg == NULL) {fputs ("   Memory error",stderr); exit (2);}

    // copy the file into the buffer:
    resultogg = fread (bufferogg,1,filesize,pFile);
    if (resultogg != filesize) {fputs ("   Reading error",stderr); exit (3);}

	fclose (pFile);

	sound = (const u8 *) bufferogg;
	length = filesize;
    */
    int ret = PlayOggFromFile(path, loop);
    SetVolumeOgg(255*(volume/100.0));
    return ret;
}

void GuiSound::Play()
{
	int vol;

	switch(type)
	{
		case SOUND_PCM:
		vol = 255*(volume/100.0);
		voice = ASND_GetFirstUnusedVoice();
		if(voice >= 0)
			ASND_SetVoice(voice, VOICE_STEREO_16BIT, 48000, 0,
				(u8 *)sound, length, vol, vol, NULL);
		break;

		case SOUND_OGG:
		voice = 0;
		if(loop)
			PlayOgg(mem_open((char *)sound, length), 0, OGG_INFINITE_TIME);
		else
			PlayOgg(mem_open((char *)sound, length), 0, OGG_ONE_TIME);
		SetVolumeOgg(255*(volume/100.0));
		break;
	}
}

void GuiSound::Stop()
{
	if(voice < 0)
		return;

	switch(type)
	{
		case SOUND_PCM:
		ASND_StopVoice(voice);
		break;

		case SOUND_OGG:
		StopOgg();
		break;
	}
}

void GuiSound::Pause()
{
	if(voice < 0)
		return;

	switch(type)
	{
		case SOUND_PCM:
		ASND_PauseVoice(voice, 1);
		break;

		case SOUND_OGG:
		PauseOgg(1);
		break;
	}
}

void GuiSound::Resume()
{
	if(voice < 0)
		return;

	switch(type)
	{
		case SOUND_PCM:
		ASND_PauseVoice(voice, 0);
		break;

		case SOUND_OGG:
		PauseOgg(0);
		break;
	}
}

bool GuiSound::IsPlaying()
{
	if(ASND_StatusVoice(voice) == SND_WORKING || ASND_StatusVoice(voice) == SND_WAITING)
		return true;
	else
		return false;
}

void GuiSound::SetVolume(int vol)
{
	volume = vol;

	if(voice < 0)
		return;

	int newvol = 255*(volume/100.0);

	switch(type)
	{
		case SOUND_PCM:
		ASND_ChangeVolumeVoice(voice, newvol, newvol);
		break;

		case SOUND_OGG:
		SetVolumeOgg(255*(volume/100.0));
		break;
	}
}

void GuiSound::SetLoop(bool l)
{
	loop = l;
}

s32 GuiSound::GetPlayTime()
{
	return GetTimeOgg();
}

void GuiSound::SetPlayTime(s32 time_pos)
{
	SetTimeOgg(time_pos);
}
