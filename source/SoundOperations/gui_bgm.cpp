/****************************************************************************
 * SettingsPrompts
 * USB Loader GX 2009
 *
 * Backgroundmusic
 ***************************************************************************/
#include <sys/dirent.h>
#include "themes/CTheme.h"
#include "gui_bgm.h"
#include "menu.h"

GuiBGM::GuiBGM(const u8 *s, int l, int v) :
	GuiSound(s, l, v, 0)
{
	currentPath = NULL;
	currentPlaying = 0;
	voice = 0;
}

GuiBGM::~GuiBGM()
{
	if (currentPath) delete[] currentPath;

	ClearList();
}

bool GuiBGM::Load(const char *path)
{
	if (!path)
	{
		LoadStandard();
		return false;
	}
	if (strcmp(path, "") == 0)
	{
		LoadStandard();
		return false;
	}

	if (!GuiSound::Load(path))
	{
		LoadStandard();
		return false;
	}

	return ParsePath(path);
}

bool GuiBGM::LoadStandard()
{
	ClearList();
	if (currentPath)
	{
		delete[] currentPath;
		currentPath = NULL;
	}

	strcpy(Settings.ogg_path, "");

	bool ret = GuiSound::Load(Resources::GetFile("bg_music.ogg"), Resources::GetFileSize("bg_music.ogg"));

	if (ret) Play();

	return ret;
}

bool GuiBGM::ParsePath(const char * folderpath)
{
	ClearList();

	if (currentPath) delete[] currentPath;

	currentPath = new char[strlen(folderpath) + 1];
	strcpy(currentPath, folderpath);

	char * isdirpath = strrchr(folderpath, '.');
	if (isdirpath)
	{
		char * pathptr = strrchr(currentPath, '/');
		if (pathptr)
		{
			pathptr++;
			pathptr[0] = 0;
		}
	}

	char * LoadedFilename = strrchr(folderpath, '/') + 1;

	char filename[1024];
	struct dirent * dirent = NULL;

	DIR * dir = opendir(currentPath);
	if (dir == NULL)
	{
		LoadStandard();
		return false;
	}
	u32 counter = 0;

	while ((dirent = readdir(dir)) != 0)
	{
		snprintf(filename, sizeof(filename), dirent->d_name);

		char * fileext = strrchr(filename, '.');
		if (!fileext)
			continue;

		if (strcasecmp(fileext, ".mp3") == 0 || strcasecmp(fileext, ".ogg") == 0 ||
			strcasecmp(fileext, ".wav") == 0 || strcasecmp(fileext, ".aif") == 0)
		{
			AddEntrie(filename);

			if (strcmp(LoadedFilename, filename) == 0) currentPlaying = counter;

			counter++;
		}

	}

	closedir(dir);

	snprintf(Settings.ogg_path, sizeof(Settings.ogg_path), "%s", folderpath);

	return true;
}

void GuiBGM::AddEntrie(const char * filename)
{
	if (!filename) return;

	char * NewEntrie = new char[strlen(filename) + 1];
	strcpy(NewEntrie, filename);

	PlayList.push_back(NewEntrie);
}

void GuiBGM::ClearList()
{
	for (u32 i = 0; i < PlayList.size(); i++)
	{
		if (PlayList.at(i) != NULL)
		{
			delete[] PlayList.at(i);
			PlayList.at(i) = NULL;
		}
	}

	PlayList.clear();
}

bool GuiBGM::PlayNext()
{
	if (!currentPath) return false;

	currentPlaying++;
	if (currentPlaying >= (int) PlayList.size()) currentPlaying = 0;

	snprintf(Settings.ogg_path, sizeof(Settings.ogg_path), "%s%s", currentPath, PlayList.at(currentPlaying));

	if (!GuiSound::Load(Settings.ogg_path)) return false;

	Play();

	return true;
}

bool GuiBGM::PlayPrevious()
{
	if (!currentPath) return false;

	currentPlaying--;
	if (currentPlaying < 0) currentPlaying = PlayList.size() - 1;

	snprintf(Settings.ogg_path, sizeof(Settings.ogg_path), "%s%s", currentPath, PlayList.at(currentPlaying));

	if (!GuiSound::Load(Settings.ogg_path)) return false;

	Play();

	return true;
}

bool GuiBGM::PlayRandom()
{
	if (!currentPath) return false;

	srand(time(NULL));

	currentPlaying = rand() % PlayList.size();

	//just in case
	if (currentPlaying < 0)
		currentPlaying = PlayList.size() - 1;
	else if (currentPlaying >= (int) PlayList.size()) currentPlaying = 0;

	snprintf(Settings.ogg_path, sizeof(Settings.ogg_path), "%s%s", currentPath, PlayList.at(currentPlaying));

	if (!GuiSound::Load(Settings.ogg_path)) return false;

	Play();

	return true;
}

void GuiBGM::UpdateState()
{
	if (!IsPlaying())
	{
		if (loop == DIR_LOOP)
		{
			PlayNext();
		}
		else if (loop == RANDOM_BGM)
		{
			PlayRandom();
		}
	}
}
