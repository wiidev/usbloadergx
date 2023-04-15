#ifndef STARTUPPROCESS_H_
#define STARTUPPROCESS_H_

#include "GUI/gui.h"

class StartUpProcess
{
public:
	static int Run(int argc, char *argv[]);

private:
	StartUpProcess();
	~StartUpProcess();
	void LoadIOS(u8 ios, bool boot);
	int Execute(bool quickGameBoot);
	int FinalizeExecute();
	bool USBSpinUp();
	void TextFade(int direction);
	void SetTextf(const char *format, ...);
	void Draw();
	static int ParseArguments(int argc, char *argv[]);
	static int QuickGameBoot(const char *gameID);
	int AutobootDisc();

	bool drawCancel;
	bool drawDiscCancel;

	GuiImageData *GXImageData;
	GuiImage *background;
	GuiImage *GXImage;
	GuiText *titleTxt;
	GuiText *messageTxt;
	GuiText *versionTxt;
	GuiText *cancelTxt;
	GuiText *discCancelTxt;
	GuiButton *cancelBtn;
	GuiButton *sdmodeBtn;
	GuiTrigger *trigB;
	GuiTrigger *trigA;
};

#endif
