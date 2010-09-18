/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_sound_plugin_mpg.cpp
 *
 * by ardi 2009
 *
 * Decoder for MPEG-Audio Mpeg-1/-2 Layer I,II and III with libmad
 *
 * GUI class definitions
 ***************************************************************************/

#include <unistd.h>
#include <limits.h>
#include <asndlib.h>
#include <mad.h>
#include <string.h>
#include <new>


#include "gui_sound_decoder.h"

static inline s16 FixedToShort( mad_fixed_t Fixed )
{
    /* Clipping */
    if ( Fixed >= MAD_F_ONE )
        return( SHRT_MAX );
    if ( Fixed <= -MAD_F_ONE )
        return( -SHRT_MAX );

    Fixed = Fixed >> ( MAD_F_FRACBITS - 15 );
    return( ( s16 )Fixed );
}

#define ADMA_BUFFERSIZE         (8192)
#define DATABUFFER_SIZE         (32768)
// http://www.fr-an.de/fragen/v06/02_01.htm


class GuiSoundDecoderMPG : public GuiSoundDecoder
{
    protected:
        GuiSoundDecoderMPG( const u8 * snd, u32 len, bool snd_is_allocated )
        {
            sound           = snd;
            length          = len;
            is_allocated    = snd_is_allocated;
            // Init mad-structures
            mad_stream_init( &madStream );
            mad_stream_buffer( &madStream, sound, length );
            mad_frame_init( &madFrame );
            mad_synth_init( &madSynth );
            madSynthPcmPos = 0;
            mad_timer_reset( &madTimer );
            guardBuffer = NULL;
            is_running = false;

            // decode first Frame
            if ( DecodeFirstFrame() )
            {
                mad_synth_finish( &madSynth );
                mad_frame_finish( &madFrame );
                mad_stream_finish( &madStream );
                throw( "Stream Error" );
            }
        }
    public:
        ~GuiSoundDecoderMPG()
        {
            while ( is_running ) usleep( 50 );
            mad_synth_finish( &madSynth );
            mad_frame_finish( &madFrame );
            mad_stream_finish( &madStream );
            delete [] guardBuffer;
            if ( is_allocated ) delete [] sound;
        }
        static GuiSoundDecoder *Create( const u8 * snd, u32 len, bool snd_is_allocated )
        {
            struct mad_stream   madStream;
            struct mad_header madHeader;
            mad_stream_init( &madStream );
            mad_stream_buffer( &madStream, snd, len );
            mad_header_init( &madHeader );
            s32 ret = mad_header_decode( &madHeader, &madStream );
            if ( ret == 0 || madStream.error == MAD_ERROR_LOSTSYNC ) // LOSTSYNC in first call is ok
            {
                int i;
                for ( i = 0; i < 4 && mad_header_decode( &madHeader, &madStream ) == 0; i++ );
                if ( i == 4 )
                {
                    mad_header_finish( &madHeader );
                    mad_stream_finish( &madStream );
                    return new GuiSoundDecoderMPG( snd, len, snd_is_allocated );
                }
            }
            mad_header_finish( &madHeader );
            mad_stream_finish( &madStream );
            return NULL;
        }
        s32 GetFormat()
        {
            return MAD_NCHANNELS( &madFrame.header ) == 2 ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT;
        }
        s32 GetSampleRate()
        {
            return madFrame.header.samplerate;
        }
        /*  Read reads data from stream to buffer
            return: >0 = readed bytes;
                    0 = EOF;
                    <0 = Error;
        */
        int Read( u8 * buffer, int buffer_size )
        {
            is_running = true;
            if ( MAD_NCHANNELS( &madFrame.header ) == 2 ) // stereo
                buffer_size &= ~0x0003; // make size to a kind of 4
            else
                buffer_size &= ~0x0001; // make size to a kind of 2
            u8 *write_pos = buffer;
            u8 *write_end = buffer + buffer_size;

            for ( ;; )
            {
                for ( ; madSynthPcmPos < madSynth.pcm.length; ++madSynthPcmPos )
                {
                    if ( write_pos >= write_end )
                    {
                        is_running = false;
                        return write_pos - buffer;
                    }
                    *( ( s16* )write_pos ) = FixedToShort( madSynth.pcm.samples[0][madSynthPcmPos] ); write_pos += 2;
                    if ( MAD_NCHANNELS( &madFrame.header ) == 2 ) // stereo
                    {
                        *( ( s16* )write_pos ) = FixedToShort( madSynth.pcm.samples[1][madSynthPcmPos] ); write_pos += 2;
                    }
                }

                madStream.error = MAD_ERROR_NONE;
                if ( mad_frame_decode( &madFrame, &madStream ) )
                {
                    if ( MAD_RECOVERABLE( madStream.error ) )
                    {
                        if ( madStream.error != MAD_ERROR_LOSTSYNC || !guardBuffer )
                            continue;
                    }
                    else if ( madStream.error == MAD_ERROR_BUFLEN )
                    {
                        if ( !guardBuffer )
                        {
                            u32 guardLen = ( madStream.bufend - madStream.next_frame );
                            guardBuffer = new( std::nothrow ) u8[guardLen + MAD_BUFFER_GUARD];
                            if ( guardBuffer )
                            {
                                memcpy( guardBuffer, madStream.next_frame, guardLen );
                                memset( guardBuffer + guardLen, 0, MAD_BUFFER_GUARD );
                                mad_stream_buffer( &madStream, guardBuffer, guardLen + MAD_BUFFER_GUARD );
                                continue;
                            }
                        }
                    }
                    break;
                }
                mad_timer_add( &madTimer, madFrame.header.duration );
                mad_synth_frame( &madSynth, &madFrame );
                madSynthPcmPos = 0;
            }
            is_running = false;
            return write_pos - buffer;
        }
        int Rewind()
        {
            while ( is_running ) usleep( 50 );
            delete [] guardBuffer; guardBuffer = NULL;
            mad_stream_buffer( &madStream, sound, length );
            mad_synth_finish( &madSynth );
            mad_synth_init( &madSynth );
            madSynthPcmPos = 0;
            mad_timer_reset( &madTimer );
            // decode first Frame
            return DecodeFirstFrame();
        }
    private:
        int DecodeFirstFrame()
        {
            for ( ;; )
            {
                madStream.error = MAD_ERROR_NONE;
                if ( mad_frame_decode( &madFrame, &madStream ) )
                {
                    if ( MAD_RECOVERABLE( madStream.error ) )
                        continue;
                    else
                        return -1;
                }
                mad_timer_add( &madTimer, madFrame.header.duration );
                mad_synth_frame( &madSynth, &madFrame );
                return 0;
            }
        }
        const u8            *sound;
        u32                 length;
        bool                is_allocated;
        struct mad_stream   madStream;
        struct mad_frame    madFrame;
        struct mad_synth    madSynth;
        u16                 madSynthPcmPos;
        mad_timer_t         madTimer;
        u8                  *guardBuffer;
        bool                is_running;
};
REGISTER_GUI_SOUND_DECODER( GuiSoundDecoderMPG );
