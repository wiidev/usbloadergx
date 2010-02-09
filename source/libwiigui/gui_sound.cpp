/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_sound.cpp
 *
 * decoder modification by ardi 2009
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include <unistd.h>
#include "gecko.h"

#include "gui_sound_decoder.h"


#define BUFFER_SIZE 8192


/***************************************************************
 *
 * D E C O D E R – L I S T
 *
 *
 ***************************************************************/

GuiSoundDecoder::DecoderListEntry *GuiSoundDecoder::DecoderList = NULL;
GuiSoundDecoder::DecoderListEntry &GuiSoundDecoder::RegisterDecoder(DecoderListEntry &Decoder, GuiSoundDecoderCreate fnc) {
    if (Decoder.fnc != fnc) {
        Decoder.fnc = fnc;
        Decoder.next = DecoderList;
        DecoderList = &Decoder;
    }
    return Decoder;
}
GuiSoundDecoder *GuiSoundDecoder::GetDecoder(const u8 * snd, u32 len, bool snd_is_allocated) {
    for (DecoderListEntry *de = DecoderList; de; de=de->next) {
        GuiSoundDecoder *d = NULL;
        try {
            d = de->fnc(snd, len, snd_is_allocated);
        } catch (const char *error) {
            gprintf("%s", error);
        } catch (...) {}
        if (d) return d;
    }
    return NULL;
}


/***************************************************************
 *
 * D E C O D E R – T H R E A D
 *
 *
 ***************************************************************/

static GuiSound *GuiSoundPlayer[16] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static lwp_t GuiSoundDecoderThreadHandle	= LWP_THREAD_NULL;
static bool GuiSoundDecoderThreadRunning	= false;
static bool GuiSoundDecoderDataRquested	= false;

void *GuiSoundDecoderThread(void *args) {
    GuiSoundDecoderThreadRunning = true;
    do {
        if (GuiSoundDecoderDataRquested) {
            GuiSoundDecoderDataRquested = false;
            GuiSound **players = GuiSoundPlayer;
            for ( int i = 0; i < 16; ++i , ++players) {
                GuiSound *player = *players;
                if (player)
                    player->DecoderCallback();
            }
        }
        if (!GuiSoundDecoderDataRquested)
            usleep(50);
    } while (GuiSoundDecoderThreadRunning);
    return 0;
}

/***************************************************************
 *
 * A S N D – C A L L B A C K
 *
 *
 ***************************************************************/

void GuiSoundPlayerCallback(s32 Voice) {
    if (Voice >= 0 && Voice < 16 && GuiSoundPlayer[Voice]) {
        GuiSoundPlayer[Voice]->PlayerCallback();
        GuiSoundDecoderDataRquested = true;
    }
}

/***************************************************************
 *
 *   R A W - D E C O D E R
 *   Decoder for Raw-PCM-Datas (16bit Stereo 48kHz)
 *
 ***************************************************************/
class GuiSoundDecoderRAW : public GuiSoundDecoder {
protected:
    GuiSoundDecoderRAW(const u8 * snd, u32 len, bool snd_is_allocated) {
        pcm_start		= snd;
        is_allocated	= snd_is_allocated;
        pcm_end			= pcm_start+len;
        pos				= pcm_start;
        is_running		= false;

    }
public:
    ~GuiSoundDecoderRAW() {
        while (is_running) usleep(50);
        if (is_allocated) delete [] pcm_start;
    }
    static GuiSoundDecoder *Create(const u8 * snd, u32 len, bool snd_is_allocated) {
        try {
            return new GuiSoundDecoderRAW(snd, len, snd_is_allocated);
        } catch (...) {}
        return NULL;
    }
    s32 GetFormat() {
        return VOICE_STEREO_16BIT;
    }
    s32 GetSampleRate() {
        return 48000;
    }
    /*  Read reads data from stream to buffer
    	return:	>0 = readed bytes;
                0 = EOF;
    			<0 = Error;
    */
    int Read(u8 * buffer, int buffer_size) {
        if (pos >= pcm_end)
            return 0; // EOF

        is_running = true;
        if (pos + buffer_size > pcm_end)
            buffer_size = pcm_end-pos;
        memcpy(buffer, pos, buffer_size);
        pos += buffer_size;
        is_running = false;
        return buffer_size;
    }
    int Rewind() {
        pos = pcm_start;
        return 0;
    }
private:
    const u8		*pcm_start;
    const u8		*pcm_end;
    bool			 is_allocated;
    const u8		*pos;
    bool			is_running;

};

/***************************************************************
 *
 *   G u i S o u n d
 *
 *
 ***************************************************************/
#define GuiSoundBufferReady	0x01
#define GuiSoundBufferEOF	0x02
#define GuiSoundFinish		0x04
static int GuiSoundCount = 0;
/**
 * Constructor for the GuiSound class.
 */
GuiSound::GuiSound(const u8 *s, int l, int v/*=100*/, bool r/*=true*/, bool a/*=false*/) {
    if (GuiSoundCount++ == 0 || GuiSoundDecoderThreadHandle == LWP_THREAD_NULL) {
        LWP_CreateThread(&GuiSoundDecoderThreadHandle,GuiSoundDecoderThread,NULL,NULL,32*1024,80);
    }
    voice = -1;
    play_buffer[0]	= (u8*)memalign(32, BUFFER_SIZE*3);	// tripple-buffer first is played
    play_buffer[1]	= play_buffer[0] + BUFFER_SIZE;		//						second is waiting
    play_buffer[2]	= play_buffer[1] + BUFFER_SIZE;		//						third is decoding
    buffer_nr		= 0;			// current playbuffer
    buffer_pos		= 0;			// current idx to write in buffer
    buffer_ready	= false;
    buffer_eof		= false;
    loop			= false;		// play looped
    volume			= v;			// volume
    decoder			= NULL;
    if (play_buffer[0])			// playbuffer ok
        Load(s, l, r, a);
}
bool GuiSound::Load(const u8 *s, int l, bool r/*=false*/, bool a/*=false*/) {
    Stop();
    if (!play_buffer[0]) return false;
    GuiSoundDecoder *newDecoder = GuiSoundDecoder::GetDecoder(s, l, a);
    if (!newDecoder && r) newDecoder = GuiSoundDecoderRAW::Create(s, l, a);
    if (newDecoder) {
        delete decoder;
        decoder = newDecoder;
        return true;
    } else if (a)
        delete [] s;
    return false;
}
GuiSound::GuiSound(const char *p, int v/*=100*/) {
    if (GuiSoundCount++ == 0 || GuiSoundDecoderThreadHandle == LWP_THREAD_NULL) {
        LWP_CreateThread(&GuiSoundDecoderThreadHandle,GuiSoundDecoderThread,NULL,NULL,32*1024,80);
    }
    voice = -1;
    play_buffer[0]	= (u8*)memalign(32, BUFFER_SIZE*3);	// tripple-buffer first is played
    play_buffer[1]	= play_buffer[0] + BUFFER_SIZE;		//						second is waiting
    play_buffer[2]	= play_buffer[1] + BUFFER_SIZE;		//						third is decoding
    buffer_nr		= 0;					// current playbuffer
    buffer_pos		= 0;					// current idx to write in buffer
    buffer_ready	= false;
    buffer_eof		= false;
    loop			= false;				// play looped
    volume			= v;					// volume
    decoder			= NULL;
    if (play_buffer[0])			// playbuffer ok
        Load(p);
}
bool GuiSound::Load(const char *p) {
    Stop();							// stop playing
    if (!play_buffer[0]) return false;

    bool ret = false;
    voice = -2;						// -2 marks loading from file
    u32 filesize = 0;
    u8 *buffer = NULL;
    size_t result;

    FILE * pFile = fopen (p, "rb");
    if (pFile) {
        // get file size:
        fseek (pFile , 0 , SEEK_END);
        filesize = ftell (pFile);
        fseek (pFile , 0 , SEEK_SET);

        // allocate memory to contain the whole file:
        buffer = new(std::nothrow) u8[filesize];
        if (buffer) {
            // copy the file into the buffer:
            result = fread (buffer, 1, filesize, pFile);
            if (result == filesize)
                ret= Load(buffer, filesize, false, true);
            else
                delete [] buffer;
        }
        fclose (pFile);
    }
    return ret;
}

/**
 * Destructor for the GuiSound class.
 */
GuiSound::~GuiSound() {
    if (!loop)	while (voice >= 0) usleep(50);
    Stop();
    if (--GuiSoundCount == 0 && GuiSoundDecoderThreadHandle != LWP_THREAD_NULL) {
        GuiSoundDecoderThreadRunning = false;
        LWP_JoinThread(GuiSoundDecoderThreadHandle,NULL);
        GuiSoundDecoderThreadHandle = LWP_THREAD_NULL;
    }
    delete decoder;
    free(play_buffer[0]);
}

void GuiSound::Play() {
    Stop();									// stop playing if it played
    if (!play_buffer[0]) return;
    if (!decoder) return;					// no decoder or no play_buffer -> no playing
    // initialize the buffer
    buffer_nr		= 0; 					// allways starts with buffer 0
    buffer_pos		= 0;					// reset position
    buffer_ready	= false;
    buffer_eof		= false;
    decoder->Rewind();						// play from begin
    DecoderCallback();						// fill first buffer;
    if (!buffer_ready || buffer_eof)			// if first buffer not ready -> no play
        return;
    voice = ASND_GetFirstUnusedVoice();
    if (voice >= 0) {
        s32 vol			= (255*volume)/100;
        s32 format		= decoder->GetFormat();
        s32 samplerate	= decoder->GetSampleRate();
        s32 first_pos	= buffer_pos;
        // switch to next buffer
        buffer_nr		= 1;
        buffer_pos		= 0;
        buffer_ready	= false;
        buffer_eof		= false;
        DecoderCallback();						// fill second buffer;
        GuiSoundPlayer[voice] = this;			// activate Callbacks for this voice
        // Play the voice
        ASND_SetVoice(voice, format, samplerate, 0, play_buffer[0], first_pos, vol, vol, GuiSoundPlayerCallback);
    }
}
/*
int GuiSound::PlayOggFile(char * path)
{
	if(Load(path))
		Play();
	return 1;
}
*/
void GuiSound::Stop() {
    if (voice < 0) return ;
    GuiSoundPlayer[voice] = NULL; // disable Callbacks
    SND_StopVoice(voice);
    voice = -1;
}

void GuiSound::Pause() {
    if (voice < 0) return ;
    ASND_PauseVoice(voice, 1);
}

void GuiSound::Resume() {
    if (voice < 0) return ;
    ASND_PauseVoice(voice, 0);
}

bool GuiSound::IsPlaying() {
    return voice >= 0;
}

void GuiSound::SetVolume(int vol) {
    volume = vol;
    if (voice < 0) return ;
    int newvol = 255*(volume/100.0);
    ASND_ChangeVolumeVoice(voice, newvol, newvol);
}

void GuiSound::SetLoop(bool l) {
    loop = l;
}

void GuiSound::DecoderCallback() {
    if (buffer_ready || buffer_eof) // if buffer ready or EOF -> nothing
        return;
    bool error=false;
    while (buffer_pos < BUFFER_SIZE) {
        int ret = decoder->Read(&play_buffer[buffer_nr][buffer_pos], BUFFER_SIZE-buffer_pos);
        if (ret > 0)
            buffer_pos += ret;					// ok -> fill the buffer more
        else if (ret == 0) {					// EOF from decoder
            if (loop)
                decoder->Rewind();				// if loop -> rewind and fill the buffer more
            else if (buffer_pos)
                break;							// has data in buffer -> play the buffer
            else
                buffer_eof = true;				// no data in buffer -> return EOF
            return;
        } else if (ret < 0) {					// an ERROR
            if (buffer_pos)
                break;							// has data in buffer -> play the buffer
            else if (loop) {
                if (!error) {					// if no prev error
                    decoder->Rewind();			// if loop -> rewind
                    error = true;				// set error-state
                    continue;					// and fill the buffer more
                }
                buffer_eof = true;				// has prev error -> error in first block -> return EOF
                return;
            } else {
                buffer_eof = true;				// no loop -> return EOF
                return;
            }
        }
        error = false;							// clear error-state
    }
    buffer_ready = true;
}
void GuiSound::PlayerCallback() {
    if (buffer_eof) {							// if EOF
        if (ASND_TestPointer(voice, play_buffer[(buffer_nr+2)%3])==0)	// test prev. Buffer
            Stop();
    } else if (buffer_ready) {			// if buffer ready
        if (ASND_AddVoice(voice, play_buffer[buffer_nr], buffer_pos)==SND_OK) {	// add buffer
            // next buffer
            buffer_nr	= (buffer_nr+1)%3;
            buffer_pos	= 0;
            buffer_ready= false;
            buffer_eof	= false;
        }
    }
}

