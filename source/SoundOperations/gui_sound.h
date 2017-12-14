/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#ifndef GUI_SOUND_H_
#define GUI_SOUND_H_

#include <gccore.h>

//!Sound conversion and playback. A wrapper for other sound libraries - ASND, libmad, ltremor, etc
class GuiSound
{
	public:
		//!Constructor
		//!\param sound Pointer to the sound data
		//!\param filesize Length of sound data
		GuiSound(const char * filepath);
		GuiSound(const u8 * sound, s32 filesize, int volume, int voice = -1);
		//!Destructor
		virtual ~GuiSound();
		//!Load a file and replace the old one
		virtual bool Load(const char * filepath);
		//!Load a file and replace the old one
		bool Load(const u8 * sound, s32 filesize);
		//!For quick playback of the internal soundeffects
		bool LoadSoundEffect(const u8 * snd, s32 len);
		//!Start sound playback
		void Play();
		//!Stop sound playback
		void Stop();
		//!Pause sound playback
		void Pause();
		//!Resume sound playback
		void Resume();
		//!Checks if the sound is currently playing
		//!\return true if sound is playing, false otherwise
		bool IsPlaying();
		//!Rewind the music
		void Rewind();
		//!Set sound volume
		//!\param v Sound volume (0-100)
		void SetVolume(int v);
		//!\param l Loop (true to loop)
		virtual void SetLoop(u8 l);
	protected:
		//!Stops sound and frees all memory/closes files
		void FreeMemory();
		u8 * sound; //!< Pointer to the sound data
		int length; //!< Length of sound data
		s32 voice; //!< Currently assigned ASND voice channel
		int volume; //!< Sound volume (0-100)
		u8 loop; //!< Loop sound playback
		u32 SoundEffectLength; //!< Check if it is an app soundeffect for faster playback
};

#endif
