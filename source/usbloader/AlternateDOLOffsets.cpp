#include "prompts/PromptWindows.h"
#include "language/gettext.h"
#include "gecko.h"

static int defaultDolSelected = 0;

void defaultDolPrompt(const char *gameid)
{
	char id[7];
	snprintf(id, sizeof(id), gameid);
	defaultDolSelected = 0;

	//Metroid Prime Trilogy
	if (strcmp(id, "R3ME01") == 0)
	{
		int choice = WindowPrompt(tr( "Select a DOL" ), 0, "Metroid Prime", "Metroid Prime 2", "Metroid Prime 3", tr("Pick from a list"));
		if(choice == 1)
			defaultDolSelected = 780;

		else if(choice == 2)
			defaultDolSelected = 781;

		else if(choice == 3)
			defaultDolSelected = 782;
	}
	//Metroid Prime Trilogy
	else if (strcmp(id, "R3MP01") == 0)
	{
		int choice = WindowPrompt(tr( "Select a DOL" ), 0, "Metroid Prime", "Metroid Prime 2", "Metroid Prime 3", tr("Pick from a list"));
		if(choice == 1)
			defaultDolSelected = 782;

		else if(choice == 2)
			defaultDolSelected = 783;

		else if(choice == 3)
			defaultDolSelected = 784;
	}
}

int defaultAltDol(const char *gameid)
{
	//! If one dol was selected in the defaultDolPrompt on game start
	//! return that dol offset
	if(defaultDolSelected)
		return defaultDolSelected;

	char id[7];
	snprintf(id, sizeof(id), gameid);

	//Boogie
	if (strcmp(id, "RBOP69") == 0) return 675;//previous value was 657
	if (strcmp(id, "RBOE69") == 0) return 675;//starstremr

	//Fifa 08
	if (strcmp(id, "RF8E69") == 0) return 439;//from isostar
	if (strcmp(id, "RF8P69") == 0) return 463;//from isostar
	if (strcmp(id, "RF8X69") == 0) return 464;//from isostar

	//Madden NFL07
	if (strcmp(id, "RMDP69") == 0) return 39;//from isostar

	//Madden NFL08
	if (strcmp(id, "RNFP69") == 0) return 1079;//from isostar

	//Medal of Honor: Heroes 2
	if (strcmp(id, "RM2X69") == 0) return 601;//dj_skual
	if (strcmp(id, "RM2P69") == 0) return 517;//MZottel
	if (strcmp(id, "RM2E69") == 0) return 492;//Old8oy

	//Mortal Kombat
	if (strcmp(id, "RKMP5D") == 0) return 290;//from isostar
	if (strcmp(id, "RKME5D") == 0) return 290;//starstremr

	//NBA 08
	if (strcmp(id, "RNBX69") == 0) return 964;//from isostar

	//Pangya! Golf with Style
	if (strcmp(id, "RPYP9B") == 0) return 12490;//from isostar

	//Redsteel
	if (strncmp(id, "RED", 3) == 0) return 1957;//from isostar

	//SSX
	if (strcmp(id, "RSXP69") == 0) return 377;//previous value was 337
	if (strcmp(id, "RSXE69") == 0) return 377;//previous value was 337

	//Madden NFL 07
	if (strcmp(id, "RMDE69") == 0) return 39; //from TwEbErIs

	//Madden NFL 08
	if (strcmp(id, "RNFE69") == 0) return 1079; //from TwEbErIs

	//Super Swing Golf
	if (strcmp(id, "RPYE9B") == 0) return 10617; //from TwEbErIs

	//NBA Live 08
	if (strcmp(id, "RNBE69") == 0) return 936; //from TwEbErIs

	return 0; //none found
}

int autoSelectDolPrompt(const char *gameid)
{
	char id[7];
	snprintf(id, sizeof(id), gameid);

	//Indiana Jones and the Staff of Kings (Fate of Atlantis)
	if (strcmp(id, "RJ8E64") == 0 || strcmp(id, "RJ8P64") == 0)
	{
		int choice = WindowPrompt(tr( "Select a DOL" ), 0, "Fate of Atlantis", tr("Pick from a list"), tr( "Cancel" ));
		if(choice == 1)
			return 8; //from starstremr
		else if(choice == 0)
			return 0;
	}
	//Metal Slug Anthology (Metal Slug 6)
	else if (strcmp(id, "RMLEH4") == 0)
	{
		int choice = WindowPrompt(tr( "Select a DOL" ), 0, "Metal Slug 6", tr("Pick from a list"), tr( "Cancel" ));
		if(choice == 1)
			return 54;
		else if(choice == 0)
			return 0;
	}
	//Metal Slug Anthology (Metal Slug 6)
	else if (strcmp(id, "RMLP7U") == 0)
	{
		int choice = WindowPrompt(tr( "Select a DOL" ), 0, "Metal Slug 6", tr("Pick from a list"), tr( "Cancel" ));
		if(choice == 1)
			return 56;
		else if(choice == 0)
			return 0;
	}
	//Rampage: Total Destruction (M1.dol=Rampage, jarvos.dol=Rampage World Tour)
	else if (strcmp(id, "RPGP5D") == 0)
	{
		int choice = WindowPrompt(tr( "Select a DOL" ), 0, "Rampage", "World Tour", tr("Pick from a list"), tr( "Cancel" ));
		if(choice == 1)
			return 369;

		else if(choice == 2)
			return 368;

		else if(choice == 0)
			return 0;
	}
	//The House Of The Dead 2 & 3 Return (only to play 2)
	else if (strcmp(id, "RHDE8P") == 0 || strcmp(id, "RHDP8P") == 0)
	{
		int choice = WindowPrompt(tr( "Select a DOL" ), 0, "HotD 2", tr("Pick from a list"), tr( "Cancel" ));
		if(choice == 1)
			return 149;
		else if(choice == 2)
			return 0;
		else if(choice == 0)
			return 0;
	}
	//Grand Slam Tennis
	else if (strcmp(id, "R5TP69") == 0 || strcmp(id, "R5TE69") == 0)
	{
		int choice = WindowPrompt(tr( "Select a DOL" ), 0, tr("Motion+ Video"), tr("Pick from a list"), tr( "Cancel" ));
		if(choice == 1)
			return 1493;//from isostar
		else if(choice == 0)
			return 0;
	}
	//Medal of Honor Heroes
	else if (strcmp(id, "RMZX69") == 0 || strcmp(id, "RMZP69") == 0 || strcmp(id, "RMZE69") == 0)
	{
		int choice = WindowPrompt(tr( "Select a DOL" ), 0, tr("Motion+ Video"), tr("Pick from a list"), tr( "Cancel" ));
		if(choice == 1)
			return 492;//from isostar
		else if(choice == 0)
			return 0;
	}
	//Tiger Woods 10
	else if(strcmp(id, "R9OP69") == 0)
	{
		int choice = WindowPrompt(tr( "Select a DOL" ), 0, tr("Motion+ Video"), tr("Pick from a list"), tr( "Cancel" ));
		if(choice == 1)
			return 1991;//from isostar
		else if(choice == 0)
			return 0;
	}
	//Tiger Woods 10
	else if(strcmp(id, "R9OE69") == 0)
	{
		int choice = WindowPrompt(tr( "Select a DOL" ), 0, tr("Motion+ Video"), tr("Pick from a list"), tr( "Cancel" ));
		if(choice == 1)
			return 1973;//starstremr
		else if(choice == 0)
			return 0;
	}
	//The Legend of Zelda - Skyward Sword
	else if (strcmp(id, "SOUE01") == 0)
	{
		int choice = WindowPrompt(tr( "Select a DOL" ), 0, tr("Motion+ Video"), tr("Pick from a list"), tr( "Cancel" ));
		if(choice == 1)
			return 836;//from cheatfreak47
	}
	//The Legend of Zelda - Skyward Sword
	else if (strcmp(id, "SOUP01") == 0)
	{
		int choice = WindowPrompt(tr( "Select a DOL" ), 0, tr("Motion+ Video"), tr("Pick from a list"), tr( "Cancel" ));
		if(choice == 1)
			return 974;//from Cyan
	}
	//Virtual Tennis 2009
	else if (strcmp(id, "RVUP8P") == 0)
	{
		int choice = WindowPrompt(tr( "Select a DOL" ), 0, tr("Motion+ Video"), tr("Pick from a list"), tr( "Cancel" ));
		if(choice == 1)
			return 16426;//from isostar
	}
	//Virtual Tennis 2009
	else if (strcmp(id, "RVUE8P") == 0)
	{
		int choice = WindowPrompt(tr( "Select a DOL" ), 0, tr("Motion+ Video"), tr("Pick from a list"), tr( "Cancel" ));
		if(choice == 1)
			return 16405;//from isostar
		else if(choice == 0)
			return 0;
	}
	//Wii Sports Resort
	else if (strcmp(id, "RZTP01") == 0)
	{
		int choice = WindowPrompt(tr( "Select a DOL" ), 0, tr("Motion+ Video"), tr("Pick from a list"), tr( "Cancel" ));
		if(choice == 1)
			return 952;//from isostar
		else if(choice == 0)
			return 0;
	}
	//Wii Sports Resort
	else if (strcmp(id, "RZTE01") == 0)
	{
		int choice = WindowPrompt(tr( "Select a DOL" ), 0, tr("Motion+ Video"), tr("Pick from a list"), tr( "Cancel" ));
		if(choice == 1)
			return 674;//from starstremr
		else if(choice == 0)
			return 0;
	}
	//Red Steel 2
	else if (strcmp(id, "RD2X41") == 0)
	{
		int choice = WindowPrompt(tr( "Select a DOL" ), 0, tr("Motion+ Video"), tr("Pick from a list"), tr( "Cancel" ));
		if(choice == 1)
			return 301;//from Cyan
		else if(choice == 0)
			return 0;
	}

	return -1;
}
