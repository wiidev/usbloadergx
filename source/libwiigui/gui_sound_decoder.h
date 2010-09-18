/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_sound_decoder.h
 *
 * by ardi 2009
 *
 * GUI class definitions
 ***************************************************************************/

#ifndef GUI_SOUND_DECODER_H
#define GUI_SOUND_DECODER_H

#include <gccore.h>

#define REGISTER_GUI_SOUND_DECODER(decoder) GuiSoundDecoder::DecoderListEntry decoder##_l = GuiSoundDecoder::RegisterDecoder(decoder##_l, decoder::Create)
class GuiSoundDecoder;
typedef GuiSoundDecoder *( *GuiSoundDecoderCreate )( const u8 * snd, u32 len, bool snd_is_allocated );

class GuiSoundDecoder
{
    protected:
        GuiSoundDecoder() {}; // Constructors must protected so it can create only with Init(...);
    public:
        virtual ~GuiSoundDecoder() {};
        // begin API
        // ---------
        // each Decoder must have an own static Create(...) fnc
        // static GuiSoundDecoder *Create(const u8 * snd, u32 len, bool snd_is_allocated);
        virtual s32 GetFormat() = 0;
        virtual s32 GetSampleRate() = 0;
        /*  Read reads data from stream to buffer
            return: >0 = readed bytes;
                    0 = EOF;
                    <0 = Error;
        */
        virtual int Read( u8 * buffer, int buffer_size ) = 0;
        // set the stream to the start
        virtual int Rewind() = 0;
        // -------
        // end API


        struct DecoderListEntry
        {
            GuiSoundDecoderCreate    fnc;
            DecoderListEntry        *next;
        };
        static DecoderListEntry &RegisterDecoder( DecoderListEntry &Decoder, GuiSoundDecoderCreate fnc );
        static GuiSoundDecoder *GetDecoder( const u8 * snd, u32 len, bool snd_is_allocated );
    private:
        static DecoderListEntry *DecoderList;
        GuiSoundDecoder( GuiSoundDecoder& );            // no copy
};

#define BIG_ENDIAN_HOST 1   // Wii PPC is a Big-Endian-Host

#if BIG_ENDIAN_HOST

inline uint16_t be16( const uint8_t *p8 )
{
    return *( ( uint16_t* )p8 );
}
inline uint32_t be32( const uint8_t *p8 )
{
    return *( ( uint32_t* )p8 );
}
inline uint16_t le16( const uint8_t *p8 )
{
    uint16_t ret = p8[1] << 8 | p8[0];
    return ret;
}
inline uint32_t le32( const uint8_t *p8 )
{
    uint32_t ret = p8[3] << 24 | p8[2] << 16 | p8[1] << 8 | p8[0];
    return ret;
}

#elif LITTLE_ENDIAN_HOST
inline uint16_t be16( const uint8_t *p8 )
{
    uint16_t ret = p8[0] << 8 | p8[1];
    return ret;
}
inline uint32_t be32( const uint8_t *p8 )
{
    uint32_t ret = p8[0] << 24 | p8[1] << 16 | p8[2] << 8 | p8[3];
    return ret;
}
inline uint16_t le16( const uint8_t *p8 )
{
    return *( ( uint16_t* )p8 );
}
inline uint32_t le32( const uint8_t *p8 )
{
    return *( ( uint32_t* )p8 );
}
#else
#error "BIG_ENDIAN_HOST or LITTLE_ENDIAN_HOST not setted"
#endif /* XXX_ENDIAN_HOST */

#define be16inc(p8) (p8+=2, be16(p8-2))
#define le16inc(p8) (p8+=2, le16(p8-2))
#define be32inc(p8) (p8+=4, be32(p8-4))
#define le32inc(p8) (p8+=4, le32(p8-4))



#endif /* GUI_SOUND_DECODER_H */
