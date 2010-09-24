#ifndef _GAME_SETTINGS_H_
#define _GAME_SETTINGS_H_

#include <string>
#include <stdio.h>
#include <gctypes.h>
#include <vector>
#include "usbloader/disc.h"

typedef struct _GameCFG
{
    char id[7];
    u8 video;
    u8 language;
    u8 ocarina;
    u8 vipatch;
    u8 ios;
    u8 parentalcontrol;
    u8 errorfix002;
    u8 iosreloadblock;
    u8 loadalternatedol;
    u32 alternatedolstart;
    u8 patchcountrystrings;
    char alternatedolname[40];
    u8 returnTo;
} GameCFG;

class CGameSettings
{
    public:
        //!Constructor
        CGameSettings();
        //!Destructor
        ~CGameSettings();
        //!Load
        bool Load(const char * path);
        //!Save
        bool Save();
        //!AddGame
        bool AddGame(const GameCFG * NewGame);
        //!Reset
        bool RemoveAll();
        //!Overload Reset for one Game
        bool Remove(const char * id);
        bool Remove(const u8 * id) { return Remove((const char *) id); };
        bool Remove(const struct discHdr * game) { if(!game) return false; else return Remove(game->id); };
        //!Get GameCFG
        GameCFG * GetGameCFG(const char * id);
        //!Overload
        GameCFG * GetGameCFG(const u8 * id) { return GetGameCFG((const char *) id); };
        //!Overload
        GameCFG * GetGameCFG(const struct discHdr * game) { if(!game) return NULL; else return GetGameCFG(game->id); };

    protected:
        bool ReadGameID(const char * src, char * GameID, int size);
        bool SetSetting(GameCFG & game, char *name, char *value);
        //!Find the config file in the default paths
        bool FindConfig();

        void ParseLine(char *line);
        void TrimLine(char *dest, const char *src, int size);
        std::string ConfigPath;
        std::vector<GameCFG> GameList;
};

extern CGameSettings GameSettings;

#endif
