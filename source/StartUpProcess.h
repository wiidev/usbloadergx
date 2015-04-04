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
		int Execute();
		bool USBSpinUp();
		void TextFade(int direction);
		void SetTextf(const char * format, ...);
		void Draw();
		static int ParseArguments(int argc, char *argv[]);
		static int QuickGameBoot(const char * gameID);

		bool drawCancel;

		GuiImageData * GXImageData;
		GuiImage * background;
		GuiImage * GXImage;
		GuiText * titleTxt;
		GuiText * messageTxt;
		GuiText * versionTxt;
		GuiText * cancelTxt;
		GuiButton * cancelBtn;
		GuiTrigger * trigB;
};

#endif
