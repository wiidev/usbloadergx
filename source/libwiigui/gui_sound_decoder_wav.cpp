/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_sound_plugin_wav.cpp
 *
 * by ardi 2009
 *
 * Decoder for WAVE PCM
 *
 * GUI class definitions
 ***************************************************************************/

#include <asndlib.h>
#include <map>
#include <vector>
#include <unistd.h>

#include "gui_sound_decoder.h"

typedef struct
{
    u32 cueID;
    u32 len;
    u32 loops;
} plst_t;
typedef struct
{
    const u8    *start;
    const u8    *end;
    u32         loops;
} playlist_t;

class GuiSoundDecoderWAV : public GuiSoundDecoder
{
    protected:
        GuiSoundDecoderWAV( const u8 * snd, u32 len, bool snd_is_allocated )
        {
            sound           = snd;
            is_running      = false;
            is_allocated    = snd_is_allocated;

            const u8 *in_ptr = sound;

            if ( be32inc( in_ptr ) != 0x52494646 /*'RIFF' (WAV)*/ ) throw( "Not a WAV" );

            u32 riffsize = le32inc( in_ptr );
            if ( riffsize > ( len - 8 ) ) throw( "Wrong size" );

            if ( be32inc( in_ptr ) != 0x57415645 /*'WAVE'*/ ) throw( "No  WAVE-Tag" );

            if ( be32inc( in_ptr ) != 0x666D7420 /*'fmt '*/ ) throw( "No fmt-Tag" );

            u32 fmtLen = le32inc( in_ptr );

            if ( le16inc( in_ptr ) != 1 ) throw( "Not PCM data" );

            channelCount    = le16inc( in_ptr );
            if ( channelCount < 1 || channelCount > 2 ) throw( "only mono or stereo" );

            sampleRate      = le32inc( in_ptr );

            in_ptr += 6;                    // skip <bytes/second> and <block align>

            bytePerSample   = ( le16inc( in_ptr ) + 7 ) / 8;
            if ( bytePerSample < 1 || bytePerSample > 2 ) throw( "only 1-16 bit/Sample" );

            in_ptr += fmtLen - 16;

            if ( be32inc( in_ptr ) != 0x64617461 /*'data'*/ ) throw( "No data-Tag" );


            soundDataStart = in_ptr + 4;
            soundDataEnd = soundDataStart + le32( in_ptr );

            in_ptr = soundDataEnd;

            std::map<u32, u32> cue;
            std::vector<plst_t>plst;

            if ( ( ( u32 )in_ptr ) & 0x0001UL ) in_ptr++;
            while ( ( in_ptr + 4 ) < ( sound + riffsize ) )
            {
                u32 tag = be32inc( in_ptr );
                switch ( tag )
                {
                    case 0x63756520 /*'cue '*/:
                        in_ptr += 4;                    // skip size
                        for ( u32 count = le32inc( in_ptr ); count > 0; count-- )
                        {
                            u32 ID = be32inc( in_ptr );
                            in_ptr += 4;                // skip dwPosition
                            if ( be32inc( in_ptr ) == 0x64617461 /*'data'*/ )
                            {
                                in_ptr += 8;            // skip chunkStart - dwBlockStart
                                cue[ID] = le32inc( in_ptr );
                            }
                            else
                                in_ptr += 12;           // skip chunkStart - SammpleOffset
                        }
                        break;
                    case 0x706C7374 /*' plst'*/:
                        in_ptr += 4;                    // skip size
                        for ( u32 count = le32inc( in_ptr ); count > 0; count-- )
                            plst.push_back( ( plst_t ) {le32inc( in_ptr ), le32inc( in_ptr ), le32inc( in_ptr )} );
                        break;
                    default:
                        in_ptr -= 2;
                        break;
                }
            }
            for ( std::vector<plst_t>::iterator i = plst.begin(); i != plst.end(); ++i )
            {
                const u8 *start = soundDataStart + cue[i->cueID];
                const u8 *end   = soundDataStart + ( i->len * bytePerSample * channelCount );
                u32 loops       = i->loops;
                playlist.push_back( ( playlist_t ) {start, end, loops} );
            }
            if ( playlist.size() == 0 )
            {
                playlist.push_back( ( playlist_t ) {soundDataStart, soundDataEnd, 1} );
            }
            Rewind();
        }
    public:
        ~GuiSoundDecoderWAV()
        {
            while ( is_running ) usleep( 50 );
            if ( is_allocated ) delete [] sound;
        }
        static GuiSoundDecoder *Create( const u8 * snd, u32 len, bool snd_is_allocated )
        {
            if ( snd && len > 4 && snd[0] == 'R' && snd[1] == 'I' && snd[2] == 'F' && snd[3] == 'F'
                    && snd[8] == 'W' && snd[9] == 'A' && snd[10] == 'V' && snd[11] == 'E' )
                return new GuiSoundDecoderWAV( snd, len, snd_is_allocated );
            return NULL;
        }
        s32 GetFormat()
        {
            if ( bytePerSample == 2 )
                return channelCount == 2 ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT;
            else
                return channelCount == 2 ? VOICE_STEREO_8BIT : VOICE_MONO_8BIT;
        }
        s32 GetSampleRate()
        {
            return sampleRate;
        }
        /*  Read reads data from stream to buffer
            return: >0 = readed bytes;
                    0 = EOF;
                    <0 = Error;
        */
        int Read( u8 * buffer, int buffer_size )
        {
            is_running = true;
            u8 *write_pos = buffer;
            u8 *write_end = buffer + buffer_size;

            for ( ;; )
            {
                while ( currentPos < currentEnd )
                {
                    if ( write_pos >= write_end )
                    {
                        is_running = false;
                        return write_pos - buffer;
                    }
                    if ( bytePerSample == 2 )
                    {
                        *( ( s16* )write_pos ) = le16inc( currentPos );
                        write_pos += 2;
                        if ( channelCount == 2 ) // stereo
                        {
                            *( ( s16* )write_pos ) = le16inc( currentPos );
                            write_pos += 2;
                        }
                    }
                    else
                    {
                        *write_pos++ = *currentPos++;
                        if ( channelCount == 2 ) // stereo
                            *write_pos++ = *currentPos++;
                    }
                }
                if ( currentLoops > 1 )
                {
                    currentLoops--;
                    currentPos = currentStart;
                    continue;
                }
                if ( currentPlaylist != playlist.end() )
                    currentPlaylist++;
                if ( currentPlaylist != playlist.end() )
                {
                    currentStart    = currentPos = currentPlaylist->start;
                    currentEnd      = currentPlaylist->end;
                    currentLoops    = currentPlaylist->loops;
                    continue;
                }
                else
                {
                    is_running = false;
                    return write_pos - buffer;
                }
            }
            is_running = false;
            return 0;
        }
        int Rewind()
        {
            currentPlaylist = playlist.begin();
            currentStart    = currentPos = currentPlaylist->start;
            currentEnd      = currentPlaylist->end;
            currentLoops    = currentPlaylist->loops;
            return 0;
        }
    private:
        const u8        *sound;
        bool            is_allocated;
        bool            is_running;

        u16             channelCount;
        u32             sampleRate;
        u16             bytePerSample;
        const u8        *soundDataStart;
        const u8        *soundDataEnd;
        std::vector<playlist_t> playlist;
        std::vector<playlist_t>::iterator currentPlaylist;
        const u8        *currentStart;
        const u8        *currentEnd;
        u32             currentLoops;
        const u8        *currentPos;

};
REGISTER_GUI_SOUND_DECODER( GuiSoundDecoderWAV );
