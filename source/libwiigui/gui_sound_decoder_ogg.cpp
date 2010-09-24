/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_sound_plugin_ogg.cpp
 *
 * by ardi 2009
 *
 * Decoder for ogg-vorbis with libtremor
 *
 * GUI class definitions
 ***************************************************************************/

#include <unistd.h>
#include <asndlib.h>
#include <tremor/ivorbiscodec.h>
#include <tremor/ivorbisfile.h>
#include <string.h>

#include "gui_sound_decoder.h"

/* functions to read the Ogg file from memory */

static struct
{
        char *mem;
        int size;
        int pos;
} file[4];

static int f_read(void * punt, int bytes, int blocks, int *f)
{
    int b;
    int c = 0;
    int d;

    if (bytes * blocks <= 0) return 0;

    blocks *= bytes;

    while (blocks > 0)
    {
        b = blocks;
        if (b > 4096) b = 4096;

        d = (*f) - 0x666;
        if ((unsigned) (d) <= (0x669 - 0x666))
        {
            if (file[d].size == 0) return -1;
            if ((file[d].pos + b) > file[d].size) b = file[d].size - file[d].pos;
            if (b > 0)
            {
                memcpy(punt, file[d].mem + file[d].pos, b);
                file[d].pos += b;
            }
        }
        else b = read(*f, ((char *) punt) + c, b);

        if (b <= 0)
        {
            return c / bytes;
        }
        c += b;
        blocks -= b;
    }
    return c / bytes;
}

static int f_seek(int *f, ogg_int64_t offset, int mode)
{
    if (f == NULL) return (-1);

    int k;
    mode &= 3;

    int d = (*f) - 0x666;
    if ((unsigned) (d) <= (0x669 - 0x666))
    {
        k = 0;

        if (file[d].size == 0) return -1;

        if (mode == 0)
        {
            if ((offset) >= file[d].size)
            {
                file[d].pos = file[d].size;
                k = -1;
            }
            else if ((offset) < 0)
            {
                file[d].pos = 0;
                k = -1;
            }
            else file[d].pos = offset;
        }
        else if (mode == 1)
        {
            if ((file[d].pos + offset) >= file[d].size)
            {
                file[d].pos = file[d].size;
                k = -1;
            }
            else if ((file[d].pos + offset) < 0)
            {
                file[d].pos = 0;
                k = -1;
            }
            else file[d].pos += offset;
        }
        else if (mode == 2)
        {

            if ((file[d].size + offset) >= file[d].size)
            {
                file[d].pos = file[d].size;
                k = -1;
            }
            else if ((file[d].size + offset) < 0)
            {
                file[d].pos = 0;
                k = -1;
            }
            else file[d].pos = file[d].size + offset;
        }

    }
    else k = lseek(*f, (int) offset, mode);

    if (k < 0)
        k = -1;
    else k = 0;
    return k;
}

static int f_close(int *f)
{
    int d = (*f) - 0x666;
    if ((unsigned) (d) <= (0x669 - 0x666))
    {
        file[d].size = 0;
        file[d].pos = 0;
        if (file[d].mem)
        {
            file[d].mem = (char *) 0;
        }
        return 0;
    }
    else return close(*f);
    return 0;
}

static long f_tell(int *f)
{
    int k;

    int d = (*f) - 0x666;
    if ((unsigned) (d) <= (0x669 - 0x666))
    {
        k = file[d].pos;
    }
    else k = lseek(*f, 0, 1);

    return (long) k;
}

static int mem_open(char * ogg, int size)
{
    static int one = 1;
    int n;
    if (one)
    {
        one = 0;

        file[0].size = 0;
        file[1].size = 0;
        file[2].size = 0;
        file[3].size = 0;
        file[0].mem = ogg;
        file[0].size = size;
        file[0].pos = 0;
        return (0x666);
    }

    for (n = 0; n < 4; n++)
    {
        if (file[n].size == 0)
        {
            file[n].mem = ogg;
            file[n].size = size;
            file[n].pos = 0;
            return (0x666 + n);
        }
    }
    return -1;
}

static int mem_close(int fd)
{
    if ((unsigned) ((fd) - 0x666) <= (0x669 - 0x666)) // it is a memory file descriptor?
    {
        fd -= 0x666;
        file[fd].size = 0;
        return 0;
    }
    else return f_close(&fd);
}

static ov_callbacks callbacks = { (size_t(*)(void *, size_t, size_t, void *)) f_read,
        (int(*)(void *, ogg_int64_t, int)) f_seek, (int(*)(void *)) f_close, (long(*)(void *)) f_tell };

class GuiSoundDecoderOGG: public GuiSoundDecoder
{
    protected:
        GuiSoundDecoderOGG(const u8 * snd, u32 len, bool snd_is_allocated)
        {
            sound = snd;
            is_allocated = snd_is_allocated;
            ogg_fd = mem_open((char *) snd, len);
            if (ogg_fd < 0) throw("mem open failed");

            if (ov_open_callbacks((void*) &ogg_fd, &ogg_file, NULL, 0, callbacks) < 0)
            {
                mem_close(ogg_fd);
                throw("ogg open failed");
            }
            ogg_info = ov_info(&ogg_file, -1);
            bitstream = 0;
            is_running = false;
        }
    public:
        ~GuiSoundDecoderOGG()
        {
            while (is_running)
                usleep(50);
            ov_clear(&ogg_file);
            if (is_allocated) delete[] sound;
        }
        static GuiSoundDecoder *Create(const u8 * snd, u32 len, bool snd_is_allocated)
        {
            if (snd && len > 4 && snd[0] == 'O' && snd[1] == 'g' && snd[2] == 'g' && snd[3] == 'S') return new GuiSoundDecoderOGG(
                    snd, len, snd_is_allocated);
            return NULL;
        }
        s32 GetFormat()
        {
            return ogg_info->channels == 2 ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT;
        }
        s32 GetSampleRate()
        {
            return ogg_info->rate;
        }
        /*  Read reads data from stream to buffer
         return: >0 = readed bytes;
         0 = EOF;
         <0 = Error;
         */
        int Read(u8 * buffer, int buffer_size)
        {
            is_running = true;
            int ret = ov_read(&ogg_file, (char *) buffer, buffer_size, &bitstream);
            if (ret < 0)
            {
                /* error in the stream.  Not a problem, just reporting it in
                 case we (the app) cares.  In this case, we don't. */
                if (ret != OV_HOLE) ret = 0; // we says EOF
            }
            is_running = false;
            return ret;
        }
        int Rewind()
        {
            return ov_time_seek(&ogg_file, 0);
        }
    private:
        const u8 *sound;
        bool is_allocated;
        int ogg_fd;
        OggVorbis_File ogg_file;
        vorbis_info *ogg_info;
        int bitstream;
        bool is_running;
};
REGISTER_GUI_SOUND_DECODER( GuiSoundDecoderOGG );
