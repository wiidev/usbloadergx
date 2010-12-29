#ifndef BACKGROUND_IMAGE_H_
#define BACKGROUND_IMAGE_H_

u8 * GetImageData();
void Background_Show(float x, float y, float z, u8 * data, float angle, float scaleX, float scaleY, u8 alpha);
void fadein(u8 * imgdata);
void fadeout(u8 * imgdata);

#endif
