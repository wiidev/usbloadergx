#ifndef _GUIIMAGEASYNC_H_
#define _GUIIMAGEASYNC_H_

#// arg is a pointer created with malloc()
// when the image is destroied then will also the arg deleted with free()
typedef GuiImageData * (*ImageLoaderCallback)(void *arg);

class GuiImageAsync : public GuiImage {
public:
    GuiImageAsync(const char *Filename, GuiImageData * PreloadImg);
    GuiImageAsync(ImageLoaderCallback Callback, void *arg, int arglen, GuiImageData * PreloadImg);
    ~GuiImageAsync();

private:
    GuiImageData *loadet_imgdata;
    friend 	void loader(GuiImageAsync *InUse);

    friend void Setter(GuiImageAsync *InUse);
    friend void *GuiImageAsyncThread(void *arg);
    ImageLoaderCallback callback;
    void *arg;
};



#endif /*_GUIIMAGEASYNC_H_*/
