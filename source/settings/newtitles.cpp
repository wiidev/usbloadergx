#include <stdio.h>
#include <string.h>

#include "CSettings.h"
#include "settings/CGameStatistics.h"
#include "newtitles.h"

#define NEW_SECONDS (24 * 60 * 60)
#define GAMETITLES  "GameTimestamps.txt"

NewTitles *NewTitles::instance = NULL;

NewTitles::NewTitles()
{
    firstTitle = lastTitle = NULL;
    isDirty = isNewFile = false;

    // Read the text file
    char path[255];
    snprintf(path, sizeof(path), Settings.titlestxt_path);
    if(path[strlen(path)-1] != '/')
        strcat(path, "/");

    snprintf(path, sizeof(path), "%s%s", path, GAMETITLES);

    char line[50];
    FILE *fp = fopen(path, "rb");
    if (!fp)
    {
        isNewFile = true;
        return;
    }

    time_t currenttime = time(0);

    while (fgets(line, sizeof(line), fp))
    {
        // This is one line
        if (line[0] == '#' || line[0] == ';')
            continue;

        Title *title = new Title();
        if (sscanf(line, "%6c:%lu", title->titleId, &title->timestamp) != 2)
        {
            delete title; // Invalid title entry, ignore
            continue;
        }

        title->next = NULL;
        title->isNew = ((currenttime - title->timestamp) < NEW_SECONDS);

        if (firstTitle == NULL)
        {
            firstTitle = title;
            lastTitle = title;
        }
        else
        {
            lastTitle->next = title;
            lastTitle = title;
        }
    }

    fclose(fp);
}

NewTitles::~NewTitles()
{
    Save();

    Title *t = firstTitle;
    while (t != NULL)
    {
        Title *temp = t->next;
        delete t;
        t = temp;
    }
    firstTitle = lastTitle = NULL;
}

void NewTitles::CheckGame(const u8 *titleid)
{
    if (titleid == NULL || strlen((char *) titleid) == 0)
        return;

    Title *t = firstTitle;
    while (t != NULL)
    {
        // Loop all titles, search for the correct titleid
        if (memcmp(titleid, t->titleId, 6) == 0)
            return; // Game found, which is excellent

        t = t->next;
    }

    // Not found, add it
    t = new Title;
    snprintf(t->titleId, sizeof(t->titleId), (char *) titleid);
    t->timestamp = time(0);
    t->next = NULL;

    if (isNewFile)
    {
        t->isNew = false;
        t->timestamp -= (NEW_SECONDS + 1); // Mark all games as not new if this is a new file
    }
    else
    {
        GameStatus *Status = GameStatistics.GetGameStatus(titleid);
        t->isNew = (Status == NULL || Status->PlayCount == 0);
    }

    if (firstTitle == NULL)
    {
        firstTitle = t;
        lastTitle = t;
    }
    else
    {
        lastTitle->next = t;
        lastTitle = t;
    }
    isDirty = true;
}

bool NewTitles::IsNew(const u8 *titleid) const
{
    if (!titleid) return false;

    Title *t = firstTitle;

    while(t != NULL)
    {
        // Loop all titles, search for the correct titleid
        if (memcmp(titleid, t->titleId, 6) == 0)
            return t->isNew;

        t = t->next;
    }

    return false;
}

void NewTitles::Remove(const u8 *titleid)
{
    if (titleid == NULL) return;

    Title *t = firstTitle, *prev = NULL;
    while (t != NULL)
    {
        if (memcmp(titleid, t->titleId, 6) == 0)
        {
            if (prev == NULL)
                firstTitle = t->next;
            else
                prev->next = t->next;

            delete t;
            isDirty = true;
            return;
        }
        prev = t;
        t = t->next;
    }
}

void NewTitles::Save()
{
    if (!isDirty) return;

    char path[255];
    snprintf(path, sizeof(path), Settings.titlestxt_path);
    if(path[strlen(path)-1] != '/')
        strcat(path, "/");

    snprintf(path, sizeof(path), "%s%s", path, GAMETITLES);

    FILE *fp = fopen(path, "wb");
    if (fp == NULL)
        return;

    Title *t = firstTitle;
    while (t != NULL && strlen(t->titleId) > 0)
    {
        fprintf(fp, "%s:%lu\n", t->titleId, t->timestamp);
        t = t->next;
    }
    fclose(fp);
}
