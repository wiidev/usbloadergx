#include <stdio.h>
#include <string.h>

#include "cfg.h"
#include "newtitles.h"

#define NEW_SECONDS 24 * 60 * 60
#define GAMETITLES	"gametitles.txt"

NewTitles *NewTitles::instance = NULL;

NewTitles* NewTitles::Instance()
{
	if (instance == NULL) {
		instance = new NewTitles();
	}
	return instance;
}

void NewTitles::DestroyInstance()
{
	if (instance != NULL)
	{
		delete instance;
		instance = NULL;
	}
}

NewTitles::NewTitles()
{
	firstTitle = lastTitle = NULL;
	isDirty = isNewFile = false;

	// Read the text file
	char path[255];
	strcpy(path, Settings.titlestxt_path);
	path[strlen(Settings.titlestxt_path) - 1] = '/';
	strcat(path, GAMETITLES);

	char line[20];
	FILE *fp = fopen(path, "r");
	if (fp != NULL) {
		while (fgets(line, sizeof(line), fp)) {
			// This is one line
			if (line[0] != '#' || line[0] != ';') {
				Title *title = new Title();
				if (sscanf(line, "%6c:%ld", (u8 *) &title->titleId, &title->timestamp) == 2) {
					if (firstTitle == NULL) { 
						firstTitle = title;
						lastTitle = title;
					} else {
						lastTitle->next = title;
						lastTitle = title;
					}
				}
				else {
					delete title; // Invalid title entry, ignore
				}
			}
		}
		
		fclose(fp);
	} else {
		isNewFile = true;
	}
}

NewTitles::~NewTitles()
{
	Save();
	
	Title *t = firstTitle;
	while (t != NULL) {
		Title *temp = (Title *) t->next;
		delete t;
		t = temp;
	}
	firstTitle = lastTitle = NULL;
}

void NewTitles::CheckGame(u8 *titleid)
{
	Title *t = firstTitle;
	while (t != NULL) {
		// Loop all titles, search for the correct titleid
		if (strcmp((const char *) titleid, (const char *) t->titleId) == 0) {
			return; // Game found, which is excellent
		}	
		t = (Title *) t->next;
	}

	// Not found, add it
	t = new Title();
	strncpy((char *) t->titleId, (char *) titleid, 6);
	t->timestamp = time(NULL);
	if (isNewFile) {
		t->timestamp -= (NEW_SECONDS + 1); // Mark all games as not new if this is a new file
	}

	if (firstTitle == NULL) {
		firstTitle = t;
		lastTitle = t;
	} else {
		lastTitle -> next = t;
		lastTitle = t;
	}
	isDirty = true;
}

bool NewTitles::IsNew(u8 *titleid)
{
	Title *t = firstTitle;
	
	while (t != NULL) {
		// Loop all titles, search for the correct titleid
		if (strcmp((const char *) titleid, (const char *) t->titleId) == 0) {
			// This title is less than 24 hours old
			if ((time(NULL) - t->timestamp) < NEW_SECONDS) {
				// Only count the game as new when it's never been played through GX
				Game_NUM *gnum = CFG_get_game_num(titleid);
				return gnum == NULL || gnum->count == 0;
			}
			return false;
		}	
		t = (Title *) t->next;
	}
	// We should never get here, since all files should be added by now!
	CheckGame(titleid);
	
	return !isNewFile; // If this is a new file, return false
}

void NewTitles::Remove(u8 *titleid)
{
	Title *t = firstTitle, *prev = NULL;
	while (t != NULL) {
		if (strcmp((const char *) titleid, (const char *) t->titleId) == 0) {
			if (prev == NULL) {
				firstTitle = (Title *) t->next;
			} else {
				prev->next = t->next;
			}
			delete t;
			isDirty = true;
			
			return;
		}	
		prev = t;
		t = (Title *) t->next;
	}
}

void NewTitles::Save()
{
	if (!isDirty) return;

	char path[255];
	strcpy(path, Settings.titlestxt_path);
	path[strlen(Settings.titlestxt_path) - 1] = '/';
	strcat(path, GAMETITLES);

	FILE *fp = fopen(path, "w");
	if (fp != NULL) {	
		Title *t = firstTitle;
		while (t != NULL) {
			fprintf(fp, "%s:%ld\n", t->titleId, t->timestamp);
			t = (Title *) t->next;
		}
		fclose(fp);
	}
}
