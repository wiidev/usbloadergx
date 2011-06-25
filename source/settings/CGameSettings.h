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
    short video;
    short language;
    short ocarina;
    short vipatch;
    short ios;
    short parentalcontrol;
    short errorfix002;
    short iosreloadblock;
    short loadalternatedol;
    u32 alternatedolstart;
    short patchcountrystrings;
    char alternatedolname[40];
    short returnTo;
    short sneekVideoPatch;
    short Locked;
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
        bool AddGame(const GameCFG & NewGame);
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
        //!Quick settings to PEGI conversion
        static int GetPartenalPEGI(int parentalsetting);
        //!Get the default configuration block
        GameCFG * GetDefault();
    protected:
        bool ReadGameID(const char * src, char * GameID, int size);
        bool SetSetting(GameCFG & game, char *name, char *value);
        bool ValidVersion(FILE * file);
        //!Find the config file in the default paths
        bool FindConfig();

        void ParseLine(char *line);
        void TrimLine(char *dest, const char *src, int size);
        std::string ConfigPath;
        std::vector<GameCFG> GameList;
        GameCFG DefaultConfig;
};

extern CGameSettings GameSettings;

#endif
