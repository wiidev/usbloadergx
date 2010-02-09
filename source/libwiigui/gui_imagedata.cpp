/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_imagedata.cpp
 *
 * LoadJpeg copyright by r-win for WiiXplorer
 * check WiiXplorer source for license conditions
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <jpeglib.h>

#ifdef __cplusplus
}
#endif

#define new_width 640
#define new_height 480

/**
 * Constructor for the GuiImageData class.
 */

extern int idiotFlag;
extern char idiotChar[50];
GuiImageData::GuiImageData(const u8 * img) {
    data = NULL;
    width = 0;
    height = 0;

    if (img) {
        PNGUPROP imgProp;
        IMGCTX ctx = PNGU_SelectImageFromBuffer(img);

        if (!ctx)
            return;

        int res = PNGU_GetImageProperties(ctx, &imgProp);
        //if (((4%imgProp.imgWidth)!=0)||((4%imgProp.imgHeight)!=0))idiotFlag=1;

        if (res == PNGU_OK) {
            int len = imgProp.imgWidth * imgProp.imgHeight * 4;
            if (len%32) len += (32-len%32);
            data = (u8 *)memalign (32, len);

            if (data) {
                res = PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, data, 255);

                if (res == PNGU_OK) {
                    width = imgProp.imgWidth;
                    height = imgProp.imgHeight;
                    DCFlushRange(data, len);
                } else {
                    free(data);
                    data = NULL;
                    idiotFlag=1;
                    snprintf(idiotChar, sizeof(idiotChar), "%s", img);

                }
            }
        }
        PNGU_ReleaseImageContext (ctx);
    }
}

GuiImageData::GuiImageData(const u8 * img, int imgSize) {
    data = NULL;
    width = 0;
    height = 0;

    if (img) {
        if (img[0] == 0xFF && img[1] == 0xD8) { // IMAGE_JPEG
            LoadJpeg(img, imgSize);
        }
    }
}
/**
 * Constructor for the GuiImageData class.
 */
GuiImageData::GuiImageData(const char * imgPath, const u8 * buffer) {
    data = NULL;
    width = 0;
    height = 0;

    if (imgPath) {
        PNGUPROP imgProp;
        IMGCTX ctx = PNGU_SelectImageFromDevice(imgPath);
        //if (((4%imgProp.imgWidth)!=0)||((4%imgProp.imgHeight)!=0))idiotFlag=1;

        if (ctx) {
            int res = PNGU_GetImageProperties(ctx, &imgProp);

            if (res == PNGU_OK) {
                int len = imgProp.imgWidth * imgProp.imgHeight * 4;
                if (len%32) len += (32-len%32);
                data = (u8 *)memalign (32, len);

                if (data) {
                    res = PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, data, 255);

                    if (res == PNGU_OK) {
                        width = imgProp.imgWidth;
                        height = imgProp.imgHeight;
                        DCFlushRange(data, len);
                    } else {
                        free(data);
                        data = NULL;
                        idiotFlag=1;
                        snprintf(idiotChar, sizeof(idiotChar), "%s", imgPath);
                    }
                }
            }
            PNGU_ReleaseImageContext (ctx);
        }
    }

    if (!data) { //use buffer data instead
        width = 0;
        height = 0;
        if (buffer) {
            PNGUPROP imgProp;
            IMGCTX ctx = PNGU_SelectImageFromBuffer(buffer);

            if (!ctx)
                return;

            int res = PNGU_GetImageProperties(ctx, &imgProp);

            if (res == PNGU_OK) {
                int len = imgProp.imgWidth * imgProp.imgHeight * 4;
                if (len%32) len += (32-len%32);
                data = (u8 *)memalign (32, len);

                if (data) {
                    res = PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, data, 255);

                    if (res == PNGU_OK) {
                        width = imgProp.imgWidth;
                        height = imgProp.imgHeight;
                        DCFlushRange(data, len);
                    } else {
                        free(data);
                        data = NULL;
                    }
                }
            }
            PNGU_ReleaseImageContext (ctx);
        }
    }
}

/**
 * Constructor for the GuiImageData class.
 */
GuiImageData::GuiImageData(const char *path, const char *file, const u8 *buffer, bool force_widescreen/*=false*/, const u8 *wbuffer/*=NULL*/) {
    data = NULL;
    width = 0;
    height = 0;
    char path_4_3[100];
    char path_16_9[100];
    char *imgPath;

    snprintf(path_4_3, sizeof(path_4_3), "%s%s", path, file);
    if (force_widescreen) {
        snprintf(path_16_9, sizeof(path_16_9), "%sw%s", path, file);
        imgPath = path_16_9;
        if (wbuffer)
            buffer = wbuffer;
    } else
        imgPath = path_4_3;

    for (;;) {
        if (imgPath) {
            PNGUPROP imgProp;
            IMGCTX ctx = PNGU_SelectImageFromDevice(imgPath);
            //if (((4%imgProp.imgWidth)!=0)||((4%imgProp.imgHeight)!=0))idiotFlag=1;

            if (ctx) {
                int res = PNGU_GetImageProperties(ctx, &imgProp);

                if (res == PNGU_OK) {
                    int len = imgProp.imgWidth * imgProp.imgHeight * 4;
                    if (len%32) len += (32-len%32);
                    data = (u8 *)memalign (32, len);

                    if (data) {
                        res = PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, data, 255);

                        if (res == PNGU_OK) {
                            width = imgProp.imgWidth;
                            height = imgProp.imgHeight;
                            DCFlushRange(data, len);
                        } else {
                            free(data);
                            data = NULL;
                            idiotFlag=1;
                            snprintf(idiotChar, sizeof(idiotChar), "%s", imgPath);
                        }
                    }
                }
                PNGU_ReleaseImageContext (ctx);
            }
        }
        if (data || imgPath == path_4_3)
            break;
        imgPath = path_4_3;
    }

    if (!data) { //use buffer data instead
        width = 0;
        height = 0;
        if (buffer) {
            PNGUPROP imgProp;
            IMGCTX ctx = PNGU_SelectImageFromBuffer(buffer);

            if (!ctx)
                return;

            int res = PNGU_GetImageProperties(ctx, &imgProp);

            if (res == PNGU_OK) {
                int len = imgProp.imgWidth * imgProp.imgHeight * 4;
                if (len%32) len += (32-len%32);
                data = (u8 *)memalign (32, len);

                if (data) {
                    res = PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, data, 255);

                    if (res == PNGU_OK) {
                        width = imgProp.imgWidth;
                        height = imgProp.imgHeight;
                        DCFlushRange(data, len);
                    } else {
                        free(data);
                        data = NULL;
                    }
                }
            }
            PNGU_ReleaseImageContext (ctx);
        }
    }
}

/**
 * Destructor for the GuiImageData class.
 */
GuiImageData::~GuiImageData() {
    if (data) {
        free(data);
        data = NULL;
    }
}

u8 * GuiImageData::GetImage() {
    return data;
}

int GuiImageData::GetWidth() {
    return width;
}

int GuiImageData::GetHeight() {
    return height;
}
void GuiImageData::SetGrayscale(void) {
    GXColor color;
    u32 offset, gray;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            offset = (((y >> 2)<<4)*width) + ((x >> 2)<<6) + (((y%4 << 2) + x%4 ) << 1);
            color.r = *(data+offset+1);
            color.g = *(data+offset+32);
            color.b = *(data+offset+33);

            gray = (77*color.r + 150*color.g + 28*color.b)/255;

            *(data+offset+1) = gray;
            *(data+offset+32) = gray;
            *(data+offset+33) = gray;
        }
    }
}

// This function finds it's origin in GRRLIB, which can be found here: http://code.google.com/p/grrlib/
void GuiImageData::LoadJpeg(const u8 *img, int imgSize) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    int n = imgSize;

    while (n > 1) {
        if (img[n - 1] == 0xff && img[n] == 0xd9) {
            break;
        }
        n--;
    }

    jpeg_create_decompress(&cinfo);
    cinfo.err = jpeg_std_error(&jerr);
    cinfo.progress = NULL;
    jpeg_memory_src(&cinfo, img, n);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_calc_output_dimensions(&cinfo);

    if (cinfo.output_width > new_width || cinfo.output_height > new_height) {
        float factor = (cinfo.output_width > cinfo.output_height) ? (1.0 * cinfo.output_width) / new_width : (1.0 * cinfo.output_height) / new_height;
        cinfo.scale_num = 1;
        cinfo.scale_denom = factor;
        cinfo.do_fancy_upsampling = true;
        cinfo.do_block_smoothing = false;
        cinfo.dct_method = JDCT_IFAST;
    }

    jpeg_start_decompress(&cinfo);

    int rowsize = cinfo.output_width * cinfo.num_components;
    unsigned char *tempBuffer = (unsigned char *) malloc(rowsize * cinfo.output_height);

    JSAMPROW row_pointer[1];

    row_pointer[0] = (unsigned char*) malloc(rowsize);
    size_t location = 0;
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
        memcpy(tempBuffer + location, row_pointer[0], rowsize);
        location += rowsize;
    }

    int len = ((cinfo.output_width+3)>>2)*((cinfo.output_height+3)>>2)*32*2;

    data = (u8 *) memalign(32, len);

    RawTo4x4RGBA(tempBuffer, data, cinfo.output_width, cinfo.output_height);
    DCFlushRange(data, len);

    width = cinfo.output_width;
    height = cinfo.output_height;

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(row_pointer[0]);
    free(tempBuffer);
}

/**
 * Convert a raw bmp (RGB, no alpha) to 4x4RGBA.
 * @author DragonMinded
 * @param src
 * @param dst
 * @param width
 * @param height
*/
void GuiImageData::RawTo4x4RGBA(const unsigned char *src, void *dst, const unsigned int width, const unsigned int height) {
    unsigned int block;
    unsigned int i;
    unsigned int c;
    unsigned int ar;
    unsigned int gb;
    unsigned char *p = (unsigned char*)dst;

    for (block = 0; block < height; block += 4) {
        for (i = 0; i < width; i += 4) {
            /* Alpha and Red */
            for (c = 0; c < 4; ++c) {
                for (ar = 0; ar < 4; ++ar) {
                    /* Alpha pixels */
                    *p++ = 255;
                    /* Red pixels */
                    *p++ = src[((i + ar) + ((block + c) * width)) * 3];
                }
            }

            /* Green and Blue */
            for (c = 0; c < 4; ++c) {
                for (gb = 0; gb < 4; ++gb) {
                    /* Green pixels */
                    *p++ = src[(((i + gb) + ((block + c) * width)) * 3) + 1];
                    /* Blue pixels */
                    *p++ = src[(((i + gb) + ((block + c) * width)) * 3) + 2];
                }
            }
        } /* i */
    } /* block */
}
