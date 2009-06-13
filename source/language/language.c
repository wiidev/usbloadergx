#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <ogcsys.h>

#include "usbloader/disc.h"
#include "language.h"

static char *cfg_name, *cfg_val;

char* strcopy(char *dest, char *src, int size)
{
	strncpy(dest,src,size);
	dest[size-1] = 0;
	return dest;
}

bool checkfile(char * path)
{
    FILE * f;
    f = fopen(path,"r");
    if(f) {
    fclose(f);
    return true;
    }
    fclose(f);
return false;
}

void lang_default()
{
snprintf(LANGUAGE.ok, sizeof(LANGUAGE.ok), "OK");
snprintf(LANGUAGE.addToFavorite, sizeof(LANGUAGE.addToFavorite), "Favorite");
snprintf(LANGUAGE.all, sizeof(LANGUAGE.all), "Alphabetical");
snprintf(LANGUAGE.AppLanguage, sizeof(LANGUAGE.AppLanguage), "App Language");
snprintf(LANGUAGE.t3Covers, sizeof(LANGUAGE.t3Covers), "3D Covers");
snprintf(LANGUAGE.Areyousure, sizeof(LANGUAGE.Areyousure), "Are you sure?");
snprintf(LANGUAGE.available, sizeof(LANGUAGE.available), "available");
snprintf(LANGUAGE.AutoPatch, sizeof(LANGUAGE.AutoPatch), "AutoPatch");
snprintf(LANGUAGE.Always, sizeof(LANGUAGE.Always), "0 (Always)");
snprintf(LANGUAGE.awesometool, sizeof(LANGUAGE.awesometool), "for his awesome tool");
snprintf(LANGUAGE.Back, sizeof(LANGUAGE.Back), "Back");
snprintf(LANGUAGE.Backgroundmusic, sizeof(LANGUAGE.Backgroundmusic), "Backgroundmusic");
snprintf(LANGUAGE.Backgroundmusicpath, sizeof(LANGUAGE.Backgroundmusicpath), "Backgroundmusic Path changed.");
snprintf(LANGUAGE.BacktoHBCorWiiMenu, sizeof(LANGUAGE.BacktoHBCorWiiMenu), "Back to HBC or Wii Menu");
snprintf(LANGUAGE.BacktoLoader, sizeof(LANGUAGE.BacktoLoader), "Back to Loader");
snprintf(LANGUAGE.BacktoWiiMenu, sizeof(LANGUAGE.BacktoWiiMenu), "Back to Wii Menu");
snprintf(LANGUAGE.BootStandard, sizeof(LANGUAGE.BootStandard), "Boot/Standard");
snprintf(LANGUAGE.Both, sizeof(LANGUAGE.Both), "Both");
snprintf(LANGUAGE.Cantcreatedirectory, sizeof(LANGUAGE.Cantcreatedirectory), "Can't create directory");
snprintf(LANGUAGE.Cancel, sizeof(LANGUAGE.Cancel), "Cancel");
snprintf(LANGUAGE.Cantbeformated, sizeof(LANGUAGE.Cantbeformated), "Can't be formated");
snprintf(LANGUAGE.CheckingforUpdates, sizeof(LANGUAGE.CheckingforUpdates), "Checking for Updates");
snprintf(LANGUAGE.Cantdelete, sizeof(LANGUAGE.Cantdelete), "Can't delete:");
snprintf(LANGUAGE.ClicktoDownloadCovers, sizeof(LANGUAGE.ClicktoDownloadCovers), "Click to Download Covers");
snprintf(LANGUAGE.Clock, sizeof(LANGUAGE.Clock), "Clock");
snprintf(LANGUAGE.Close, sizeof(LANGUAGE.Close), "Close");
snprintf(LANGUAGE.Continueinstallgame, sizeof(LANGUAGE.Continueinstallgame), "Continue to install game?");
snprintf(LANGUAGE.Console, sizeof(LANGUAGE.Console), "Console");
snprintf(LANGUAGE.ConsoleDefault, sizeof(LANGUAGE.ConsoleDefault), "Console Default");
snprintf(LANGUAGE.Consoleshouldbeunlockedtomodifyit, sizeof(LANGUAGE.Consoleshouldbeunlockedtomodifyit), "Console should be unlocked to modify it.");
snprintf(LANGUAGE.ConsoleLocked, sizeof(LANGUAGE.ConsoleLocked), "Console Locked");
snprintf(LANGUAGE.Controllevel, sizeof(LANGUAGE.Controllevel), "Controllevel");
snprintf(LANGUAGE.CorrectPassword, sizeof(LANGUAGE.CorrectPassword), "Correct Password");
snprintf(LANGUAGE.Couldnotinitializenetwork, sizeof(LANGUAGE.Couldnotinitializenetwork), "Could not initialize network!");
snprintf(LANGUAGE.CouldnotopenDisc, sizeof(LANGUAGE.CouldnotopenDisc), "Could not open Disc");
snprintf(LANGUAGE.CouldnotinitializeDIPmodule, sizeof(LANGUAGE.CouldnotinitializeDIPmodule), "Could not initialize DIP module!");
snprintf(LANGUAGE.CoverDownload, sizeof(LANGUAGE.CoverDownload), "Cover Download");
snprintf(LANGUAGE.CoverPath, sizeof(LANGUAGE.CoverPath), "Cover Path");
snprintf(LANGUAGE.CoverpathChanged, sizeof(LANGUAGE.CoverpathChanged), "Coverpath Changed");
snprintf(LANGUAGE.Coverpathchange, sizeof(LANGUAGE.Coverpathchange), "Coverpath change");
snprintf(LANGUAGE.Cheatcodespathchanged, sizeof(LANGUAGE.Cheatcodespathchanged), "Cheatcodes Path changed");
snprintf(LANGUAGE.Cheatcodespath, sizeof(LANGUAGE.Cheatcodespath), "Cheatcodes Path");
snprintf(LANGUAGE.count, sizeof(LANGUAGE.count), "Play Count");
snprintf(LANGUAGE.Credits, sizeof(LANGUAGE.Credits), "Credits");
snprintf(LANGUAGE.Custompaths, sizeof(LANGUAGE.Custompaths), "Custom Paths");
snprintf(LANGUAGE.DiscImages, sizeof(LANGUAGE.DiscImages), "Disc Images");
snprintf(LANGUAGE.DiscimagePath, sizeof(LANGUAGE.DiscimagePath), "Discimage Path");
snprintf(LANGUAGE.DiscpathChanged, sizeof(LANGUAGE.DiscpathChanged), "Discpath Changed");
snprintf(LANGUAGE.Discpathchange, sizeof(LANGUAGE.Discpathchange), "Discpath change");
snprintf(LANGUAGE.DiscDefault, sizeof(LANGUAGE.DiscDefault), "Disc Default");
snprintf(LANGUAGE.Display, sizeof(LANGUAGE.Display), "Display");
snprintf(LANGUAGE.Doyouwanttoformat, sizeof(LANGUAGE.Doyouwanttoformat), "Do you want to format:");
snprintf(LANGUAGE.Doyoureallywanttodelete, sizeof(LANGUAGE.Doyoureallywanttodelete), "Do you really want to delete:");
snprintf(LANGUAGE.Doyouwanttoretryfor30secs, sizeof(LANGUAGE.Doyouwanttoretryfor30secs), "Do you want to retry for 30 secs?");
snprintf(LANGUAGE.Doyouwanttoupdate, sizeof(LANGUAGE.Doyouwanttoupdate), "How do you want to update?");
snprintf(LANGUAGE.Doyouwanttochangelanguage, sizeof(LANGUAGE.Doyouwanttochangelanguage), "Do you want to change language?");
snprintf(LANGUAGE.Downloadingfile, sizeof(LANGUAGE.Downloadingfile), "Downloading file");
snprintf(LANGUAGE.DownloadBoxartimage, sizeof(LANGUAGE.DownloadBoxartimage), "Download Boxart image?");
snprintf(LANGUAGE.Downloadfinished, sizeof(LANGUAGE.Downloadfinished), "Download finished");
snprintf(LANGUAGE.Defaultgamesettings, sizeof(LANGUAGE.Defaultgamesettings), "Default Gamesettings");
snprintf(LANGUAGE.Defaultsettings, sizeof(LANGUAGE.Defaultsettings), "Default Settings");
snprintf(LANGUAGE.Default, sizeof(LANGUAGE.Default), "Default");
snprintf(LANGUAGE.diversepatches, sizeof(LANGUAGE.diversepatches), "for diverse patches");
snprintf(LANGUAGE.Error, sizeof(LANGUAGE.Error), "Error !");
snprintf(LANGUAGE.Error002fix, sizeof(LANGUAGE.Error002fix), "Error 002 fix");
snprintf(LANGUAGE.hour, sizeof(LANGUAGE.hour), "Hour");
snprintf(LANGUAGE.Homemenu, sizeof(LANGUAGE.Homemenu), "HOME Menu");
snprintf(LANGUAGE.BOOTERROR, sizeof(LANGUAGE.BOOTERROR), "BOOT ERROR");
snprintf(LANGUAGE.ErrorreadingDisc, sizeof(LANGUAGE.ErrorreadingDisc), "Error reading Disc");
snprintf(LANGUAGE.ExitUSBISOLoader, sizeof(LANGUAGE.ExitUSBISOLoader), "Exit USB Loader GX?");
snprintf(LANGUAGE.InitializingNetwork, sizeof(LANGUAGE.InitializingNetwork), "Initializing Network");
snprintf(LANGUAGE.InsertDisk, sizeof(LANGUAGE.InsertDisk), "Insert Disk");
snprintf(LANGUAGE.InsertaWiiDisc, sizeof(LANGUAGE.InsertaWiiDisc), "Insert a Wii Disc!");
snprintf(LANGUAGE.InsertaSDCardtodownloadimages, sizeof(LANGUAGE.InsertaSDCardtodownloadimages), "Insert an SD-Card to download images.");
snprintf(LANGUAGE.InsertaSDCardtosave, sizeof(LANGUAGE.InsertaSDCardtosave), "Insert an SD-Card to save.");
snprintf(LANGUAGE.InsertaSDCardtousethatoption, sizeof(LANGUAGE.InsertaSDCardtousethatoption), "Insert an SD-Card to use this option.");
snprintf(LANGUAGE.InstallRenameandDeleteareunlocked, sizeof(LANGUAGE.InstallRenameandDeleteareunlocked), "All the features of USB Loader GX are unlocked.");
snprintf(LANGUAGE.Installerror, sizeof(LANGUAGE.Installerror), "Install Error!");
snprintf(LANGUAGE.Installagame, sizeof(LANGUAGE.Installagame), "Install a game");
snprintf(LANGUAGE.Installinggame, sizeof(LANGUAGE.Installinggame), "Installing game:");
snprintf(LANGUAGE.Failedtoboot, sizeof(LANGUAGE.Failedtoboot), "Failed to boot:");
snprintf(LANGUAGE.FailedtomountfrontSDcard, sizeof(LANGUAGE.FailedtomountfrontSDcard), "Failed to mount front SD-card");
snprintf(LANGUAGE.FailedtosetUSB, sizeof(LANGUAGE.FailedtosetUSB), "Failed to set USB:");
snprintf(LANGUAGE.Failedformating, sizeof(LANGUAGE.Failedformating), "Failed formating");
snprintf(LANGUAGE.fave, sizeof(LANGUAGE.fave), "Favorites");
snprintf(LANGUAGE.filesnotfoundontheserver, sizeof(LANGUAGE.filesnotfoundontheserver), "files not found on the server!");
snprintf(LANGUAGE.Filenotfound, sizeof(LANGUAGE.Filenotfound), "File not found.");
snprintf(LANGUAGE.filesleft, sizeof(LANGUAGE.filesleft), "file(s) left");
snprintf(LANGUAGE.FlipX, sizeof(LANGUAGE.FlipX), "Flip-X");
snprintf(LANGUAGE.Force, sizeof(LANGUAGE.Force), "Force");
snprintf(LANGUAGE.Youneedtoformatapartition, sizeof(LANGUAGE.Youneedtoformatapartition), "You need to format a partition");
snprintf(LANGUAGE.Format, sizeof(LANGUAGE.Format), "Format");
snprintf(LANGUAGE.keyboard, sizeof(LANGUAGE.keyboard), "Keyboard");
snprintf(LANGUAGE.Formattingpleasewait, sizeof(LANGUAGE.Formattingpleasewait), "Formatting, please wait...");
snprintf(LANGUAGE.formated, sizeof(LANGUAGE.formated), "formatted!");
snprintf(LANGUAGE.Forhostingcovers, sizeof(LANGUAGE.Forhostingcovers), "for hosting the covers/discarts");
snprintf(LANGUAGE.Forhostingupdatefiles, sizeof(LANGUAGE.Forhostingupdatefiles), "for hosting the update files");
snprintf(LANGUAGE.free, sizeof(LANGUAGE.free), "free");
snprintf(LANGUAGE.FreeSpace, sizeof(LANGUAGE.FreeSpace), "Free Space");
snprintf(LANGUAGE.FullShutdown, sizeof(LANGUAGE.FullShutdown), "Full Shutdown");
snprintf(LANGUAGE.GameID, sizeof(LANGUAGE.GameID), "Game ID");
snprintf(LANGUAGE.Games, sizeof(LANGUAGE.Games), "Games");
snprintf(LANGUAGE.Gameisalreadyinstalled, sizeof(LANGUAGE.Gameisalreadyinstalled), "Game is already installed:");
snprintf(LANGUAGE.GameRegion, sizeof(LANGUAGE.GameRegion), "Game Region");
snprintf(LANGUAGE.GameSize, sizeof(LANGUAGE.GameSize), "Game Size");
snprintf(LANGUAGE.GoBack, sizeof(LANGUAGE.GoBack), "Go Back");
snprintf(LANGUAGE.GUISettings, sizeof(LANGUAGE.GUISettings), "GUI Settings");
snprintf(LANGUAGE.Gameload, sizeof(LANGUAGE.Gameload), "Game Load");
snprintf(LANGUAGE.HowtoShutdown, sizeof(LANGUAGE.HowtoShutdown), "How to Shutdown?");
snprintf(LANGUAGE.Language, sizeof(LANGUAGE.Language), "Game Language");
snprintf(LANGUAGE.Languagepathchanged, sizeof(LANGUAGE.Languagepathchanged), "Languagepath changed.");
snprintf(LANGUAGE.Left, sizeof(LANGUAGE.Left), "Left");
snprintf(LANGUAGE.LikeSysMenu, sizeof(LANGUAGE.LikeSysMenu), "Like SysMenu");
snprintf(LANGUAGE.LoadingincIOS, sizeof(LANGUAGE.LoadingincIOS), "Loading in cIOS249");
snprintf(LANGUAGE.ListSort, sizeof(LANGUAGE.ListSort), "Sort Game List");
snprintf(LANGUAGE.Loadingstandardlanguage, sizeof(LANGUAGE.Loadingstandardlanguage), "Loading standard language.");
snprintf(LANGUAGE.Loadingstandardmusic, sizeof(LANGUAGE.Loadingstandardmusic), "Loading standard music.");
snprintf(LANGUAGE.Locked, sizeof(LANGUAGE.Locked), "Locked");
snprintf(LANGUAGE.LockConsole, sizeof(LANGUAGE.LockConsole), "Lock Console");
snprintf(LANGUAGE.Patchcountrystrings, sizeof(LANGUAGE.Patchcountrystrings), "Patch Country Strings");
snprintf(LANGUAGE.Missingfiles, sizeof(LANGUAGE.Missingfiles), "Missing files");
snprintf(LANGUAGE.Mature, sizeof(LANGUAGE.Mature), "3 (Mature)");
snprintf(LANGUAGE.Networkiniterror, sizeof(LANGUAGE.Networkiniterror), "Network init error");
snprintf(LANGUAGE.Neither, sizeof(LANGUAGE.Neither), "Neither");
snprintf(LANGUAGE.Next, sizeof(LANGUAGE.Next), "Next");
snprintf(LANGUAGE.Nonewupdates, sizeof(LANGUAGE.Nonewupdates), "No new updates.");
snprintf(LANGUAGE.No, sizeof(LANGUAGE.No), "No");
snprintf(LANGUAGE.Nofilemissing, sizeof(LANGUAGE.Nofilemissing), "No file missing!");
snprintf(LANGUAGE.NoHDDfound, sizeof(LANGUAGE.NoHDDfound), "No HDD found!");
snprintf(LANGUAGE.NoSDcardinserted, sizeof(LANGUAGE.NoSDcardinserted), "No SD-Card inserted!");
snprintf(LANGUAGE.Nopartitionsfound, sizeof(LANGUAGE.Nopartitionsfound), "No partitions found");
snprintf(LANGUAGE.NoUSBDevice, sizeof(LANGUAGE.NoUSBDevice), "No USB Device");
snprintf(LANGUAGE.NoWBFSpartitionfound, sizeof(LANGUAGE.NoWBFSpartitionfound), "No WBFS partition found");
snprintf(LANGUAGE.NormalCovers, sizeof(LANGUAGE.NormalCovers), "Normal Covers");
snprintf(LANGUAGE.Normal, sizeof(LANGUAGE.Normal), "Normal");
snprintf(LANGUAGE.NotaWiiDisc, sizeof(LANGUAGE.NotaWiiDisc), "Not a Wii Disc");
snprintf(LANGUAGE.NoUSBDevicefound, sizeof(LANGUAGE.NoUSBDevicefound), "No USB Device found.");
snprintf(LANGUAGE.Notenoughfreespace, sizeof(LANGUAGE.Notenoughfreespace), "Not enough free space!");
snprintf(LANGUAGE.Notasupportedformat, sizeof(LANGUAGE.Notasupportedformat), "Not supported format!");
snprintf(LANGUAGE.notset, sizeof(LANGUAGE.notset), "not set");
snprintf(LANGUAGE.of, sizeof(LANGUAGE.of), "of");
snprintf(LANGUAGE.OFF, sizeof(LANGUAGE.OFF), "OFF");
snprintf(LANGUAGE.OfficialSite, sizeof(LANGUAGE.OfficialSite), "Official Site");
snprintf(LANGUAGE.ON, sizeof(LANGUAGE.ON), "ON");
snprintf(LANGUAGE.OnlyInstall, sizeof(LANGUAGE.OnlyInstall), "Only for Install");
snprintf(LANGUAGE.Parentalcontrol, sizeof(LANGUAGE.Parentalcontrol), "Parental Control");
snprintf(LANGUAGE.Partition, sizeof(LANGUAGE.Partition), "Partition");
snprintf(LANGUAGE.Password, sizeof(LANGUAGE.Password), "Password");
snprintf(LANGUAGE.PasswordChanged, sizeof(LANGUAGE.PasswordChanged), "Password Changed");
snprintf(LANGUAGE.Passwordhasbeenchanged, sizeof(LANGUAGE.Passwordhasbeenchanged), "Password has been changed");
snprintf(LANGUAGE.Passwordchange, sizeof(LANGUAGE.Passwordchange), "Password change");
snprintf(LANGUAGE.Plays, sizeof(LANGUAGE.Plays), "Play Count");
snprintf(LANGUAGE.PowerofftheWii, sizeof(LANGUAGE.PowerofftheWii), "Power off the Wii");
snprintf(LANGUAGE.Prev, sizeof(LANGUAGE.Prev), "Prev");
snprintf(LANGUAGE.PromptsButtons, sizeof(LANGUAGE.PromptsButtons), "Prompts Buttons");
snprintf(LANGUAGE.ReloadSD, sizeof(LANGUAGE.ReloadSD), "Reload SD");
snprintf(LANGUAGE.RenameGameonWBFS, sizeof(LANGUAGE.RenameGameonWBFS), "Rename Game on WBFS");
snprintf(LANGUAGE.Restart, sizeof(LANGUAGE.Restart), "Restart");
snprintf(LANGUAGE.Restarting, sizeof(LANGUAGE.Restarting), "Restarting...");
snprintf(LANGUAGE.Return, sizeof(LANGUAGE.Return), "Return");
snprintf(LANGUAGE.released, sizeof(LANGUAGE.released), "Released");
snprintf(LANGUAGE.ReturntoWiiMenu, sizeof(LANGUAGE.ReturntoWiiMenu), "Return to Wii Menu");
snprintf(LANGUAGE.Right, sizeof(LANGUAGE.Right), "Right");
snprintf(LANGUAGE.Rumble, sizeof(LANGUAGE.Rumble), "Rumble");
snprintf(LANGUAGE.QuickBoot, sizeof(LANGUAGE.QuickBoot), "Quick Boot");
snprintf(LANGUAGE.Save, sizeof(LANGUAGE.Save), "Save");
snprintf(LANGUAGE.SaveFailed, sizeof(LANGUAGE.SaveFailed), "Save Failed");
snprintf(LANGUAGE.Specialthanksto, sizeof(LANGUAGE.Specialthanksto), "Special thanks to:");
snprintf(LANGUAGE.For, sizeof(LANGUAGE.For), "for");
snprintf(LANGUAGE.theUSBLoaderandreleasingthesourcecode, sizeof(LANGUAGE.theUSBLoaderandreleasingthesourcecode), "for the USB Loader source");
snprintf(LANGUAGE.secondsleft, sizeof(LANGUAGE.secondsleft), "seconds left");
snprintf(LANGUAGE.Setasbackgroundmusic, sizeof(LANGUAGE.Setasbackgroundmusic), "Set as backgroundmusic?");
snprintf(LANGUAGE.SelectthePartition, sizeof(LANGUAGE.SelectthePartition), "Select the Partition");
snprintf(LANGUAGE.youwanttoformat, sizeof(LANGUAGE.youwanttoformat), "you want to format");
snprintf(LANGUAGE.Standard, sizeof(LANGUAGE.Standard), "Standard");
snprintf(LANGUAGE.settings, sizeof(LANGUAGE.settings), "Settings");
snprintf(LANGUAGE.Sound, sizeof(LANGUAGE.Sound), "Sound");
snprintf(LANGUAGE.ShutdowntoIdle, sizeof(LANGUAGE.ShutdowntoIdle), "Shutdown to Idle");
snprintf(LANGUAGE.ShutdownSystem, sizeof(LANGUAGE.ShutdownSystem), "Shutdown System");
snprintf(LANGUAGE.Success, sizeof(LANGUAGE.Success), "Success:");
snprintf(LANGUAGE.Successfullyinstalled, sizeof(LANGUAGE.Successfullyinstalled), "Successfully installed:");
snprintf(LANGUAGE.Successfullyupdated, sizeof(LANGUAGE.Successfullyupdated), "Successfully Updated");
snprintf(LANGUAGE.Successfullydeleted, sizeof(LANGUAGE.Successfullydeleted), "Successfully deleted:");
snprintf(LANGUAGE.SuccessfullySaved, sizeof(LANGUAGE.SuccessfullySaved), "Successfully Saved");
snprintf(LANGUAGE.SystemDefault, sizeof(LANGUAGE.SystemDefault), "System Default");
snprintf(LANGUAGE.Thanksto, sizeof(LANGUAGE.Thanksto), "Big thanks to:");
snprintf(LANGUAGE.ThemePath, sizeof(LANGUAGE.ThemePath), "ThemePath");
snprintf(LANGUAGE.ThemepathChanged, sizeof(LANGUAGE.ThemepathChanged), "Themepath Changed");
snprintf(LANGUAGE.Themepathchange, sizeof(LANGUAGE.Themepathchange), "Themepath change");
snprintf(LANGUAGE.Titlestxtpath, sizeof(LANGUAGE.Titlestxtpath), "titles.txt Path");
snprintf(LANGUAGE.Titlestxtpathchange, sizeof(LANGUAGE.Titlestxtpathchange), "Path of titles.txt change");
snprintf(LANGUAGE.TitlestxtpathChanged, sizeof(LANGUAGE.TitlestxtpathChanged), "Path of titles.txt changed.");
snprintf(LANGUAGE.Try, sizeof(LANGUAGE.Try), "Try");
snprintf(LANGUAGE.Tooltips, sizeof(LANGUAGE.Tooltips), "Tooltips");
snprintf(LANGUAGE.Timeleft, sizeof(LANGUAGE.Timeleft), "Time left:");
snprintf(LANGUAGE.updating, sizeof(LANGUAGE.updating), "Updating");
snprintf(LANGUAGE.Unlocked, sizeof(LANGUAGE.Unlocked), "Unlocked");
snprintf(LANGUAGE.UnlockConsoletousethisOption, sizeof(LANGUAGE.UnlockConsoletousethisOption), "Unlock console to use this option.");
snprintf(LANGUAGE.Unicodefix, sizeof(LANGUAGE.Unicodefix), "Unicode Fix");
snprintf(LANGUAGE.Uninstall, sizeof(LANGUAGE.Uninstall), "Uninstall");
snprintf(LANGUAGE.Updatepath, sizeof(LANGUAGE.Updatepath), "Updatepath");
snprintf(LANGUAGE.Updatepathchanged, sizeof(LANGUAGE.Updatepathchanged), "Updatepath changed.");
snprintf(LANGUAGE.Updatefailed, sizeof(LANGUAGE.Updatefailed), "Update failed");
snprintf(LANGUAGE.Updatedol, sizeof(LANGUAGE.Updatedol), "Update DOL");
snprintf(LANGUAGE.Updateall, sizeof(LANGUAGE.Updateall), "Update All");
snprintf(LANGUAGE.Updateto, sizeof(LANGUAGE.Updateto), "Update to");
snprintf(LANGUAGE.Update, sizeof(LANGUAGE.Update), "Update");
snprintf(LANGUAGE.USBLoaderisprotected, sizeof(LANGUAGE.USBLoaderisprotected), "USB Loader GX is protected");
snprintf(LANGUAGE.USBDevicenotfound, sizeof(LANGUAGE.USBDevicenotfound), "USB Device not found");
snprintf(LANGUAGE.VideoMode, sizeof(LANGUAGE.VideoMode), "Video Mode");
snprintf(LANGUAGE.VIDTVPatch, sizeof(LANGUAGE.VIDTVPatch), "VIDTV Patch");
snprintf(LANGUAGE.Volume, sizeof(LANGUAGE.Volume), "Music Volume");
snprintf(LANGUAGE.SFXVolume, sizeof(LANGUAGE.SFXVolume), "SFX Volume");
snprintf(LANGUAGE.Waiting, sizeof(LANGUAGE.Waiting), "Waiting...");
snprintf(LANGUAGE.WaitingforUSBDevice, sizeof(LANGUAGE.WaitingforUSBDevice), "Waiting for USB Device");
snprintf(LANGUAGE.WidescreenFix, sizeof(LANGUAGE.WidescreenFix), "Widescreen Fix");
snprintf(LANGUAGE.WiiMenu, sizeof(LANGUAGE.WiiMenu), "Wii Menu");
snprintf(LANGUAGE.Wiilight, sizeof(LANGUAGE.Wiilight), "Wiilight");
snprintf(LANGUAGE.WrongPassword, sizeof(LANGUAGE.WrongPassword), "Wrong Password");
snprintf(LANGUAGE.Yes, sizeof(LANGUAGE.Yes), "Yes");
snprintf(LANGUAGE.YoudonthavecIOS, sizeof(LANGUAGE.YoudonthavecIOS), "You don't have cIOS222");
snprintf(LANGUAGE.Japanese, sizeof(LANGUAGE.Japanese), "Japanese");
snprintf(LANGUAGE.German, sizeof(LANGUAGE.German), "German");
snprintf(LANGUAGE.English, sizeof(LANGUAGE.English), "English");
snprintf(LANGUAGE.French, sizeof(LANGUAGE.French), "French");
snprintf(LANGUAGE.Spanish, sizeof(LANGUAGE.Spanish), "Spanish");
snprintf(LANGUAGE.Italian, sizeof(LANGUAGE.Italian), "Italian");
snprintf(LANGUAGE.Dutch, sizeof(LANGUAGE.Dutch), "Dutch");
snprintf(LANGUAGE.SChinese, sizeof(LANGUAGE.SChinese), "SChinese");
snprintf(LANGUAGE.TChinese, sizeof(LANGUAGE.TChinese), "TChinese");
snprintf(LANGUAGE.Korean, sizeof(LANGUAGE.Korean), "Korean");
snprintf(LANGUAGE.january, sizeof(LANGUAGE.january), "Jan");
snprintf(LANGUAGE.february, sizeof(LANGUAGE.february), "Feb");
snprintf(LANGUAGE.march, sizeof(LANGUAGE.march), "Mar");
snprintf(LANGUAGE.april, sizeof(LANGUAGE.april), "Apr");
snprintf(LANGUAGE.may, sizeof(LANGUAGE.may), "May");
snprintf(LANGUAGE.june, sizeof(LANGUAGE.june), "June");
snprintf(LANGUAGE.july, sizeof(LANGUAGE.july), "July");
snprintf(LANGUAGE.august, sizeof(LANGUAGE.august), "Aug");
snprintf(LANGUAGE.september, sizeof(LANGUAGE.september), "Sept");
snprintf(LANGUAGE.october, sizeof(LANGUAGE.october), "Oct");
snprintf(LANGUAGE.november, sizeof(LANGUAGE.november), "Nov");
snprintf(LANGUAGE.december, sizeof(LANGUAGE.december), "Dec");
snprintf(LANGUAGE.developedby, sizeof(LANGUAGE.developedby), "Developed by");
snprintf(LANGUAGE.publishedby, sizeof(LANGUAGE.publishedby), "Published by");
snprintf(LANGUAGE.wififeatures, sizeof(LANGUAGE.wififeatures), "WiFi Features");
snprintf(LANGUAGE.XMLTitles, sizeof(LANGUAGE.XMLTitles), "Titles from XML");

};


struct LANGUAGE LANGUAGE;

void language_set(char *name, char *val)
{
	cfg_name = name;
	cfg_val = val;

	if (strcmp(name, "t3Covers") == 0) {
		strcopy(LANGUAGE.t3Covers, val, sizeof(LANGUAGE.t3Covers));
		return;
	}
	if (strcmp(name, "addToFavorite") == 0) {
		strcopy(LANGUAGE.addToFavorite, val, sizeof(LANGUAGE.addToFavorite));
		return;
	}
	if (strcmp(name, "all") == 0) {
		strcopy(LANGUAGE.all, val, sizeof(LANGUAGE.all));
		return;
	}
	if (strcmp(name, "Specialthanksto") == 0) {
		strcopy(LANGUAGE.Specialthanksto, val, sizeof(LANGUAGE.Specialthanksto));
		return;
	}
	if (strcmp(name, "ok") == 0) {
		strcopy(LANGUAGE.ok, val, sizeof(LANGUAGE.ok));
		return;
	}
	if (strcmp(name, "AppLanguage") == 0) {
		strcopy(LANGUAGE.AppLanguage, val, sizeof(LANGUAGE.AppLanguage));
		return;
	}
	if (strcmp(name, "Areyousure") == 0) {
		strcopy(LANGUAGE.Areyousure, val, sizeof(LANGUAGE.Areyousure));
		return;
	}
	if (strcmp(name, "AutoPatch") == 0) {
		strcopy(LANGUAGE.AutoPatch, val, sizeof(LANGUAGE.AutoPatch));
		return;
	}
	if (strcmp(name, "Always") == 0) {
		strcopy(LANGUAGE.Always, val, sizeof(LANGUAGE.Always));
		return;
	}
	if (strcmp(name, "awesometool") == 0) {
		strcopy(LANGUAGE.awesometool, val, sizeof(LANGUAGE.awesometool));
		return;
	}
	if (strcmp(name, "Back") == 0) {
		strcopy(LANGUAGE.Back, val, sizeof(LANGUAGE.Back));
		return;
	}
	if (strcmp(name, "Backgroundmusic") == 0) {
		strcopy(LANGUAGE.Backgroundmusic, val, sizeof(LANGUAGE.Backgroundmusic));
		return;
	}
	if (strcmp(name, "Backgroundmusicpath") == 0) {
		strcopy(LANGUAGE.Backgroundmusicpath, val, sizeof(LANGUAGE.Backgroundmusicpath));
		return;
	}
	if (strcmp(name, "BacktoHBCorWiiMenu") == 0) {
		strcopy(LANGUAGE.BacktoHBCorWiiMenu, val, sizeof(LANGUAGE.BacktoHBCorWiiMenu));
		return;
	}
	if (strcmp(name, "BacktoLoader") == 0) {
		strcopy(LANGUAGE.BacktoLoader, val, sizeof(LANGUAGE.BacktoLoader));
		return;
	}
	if (strcmp(name, "BacktoWiiMenu") == 0) {
		strcopy(LANGUAGE.BacktoWiiMenu, val, sizeof(LANGUAGE.BacktoWiiMenu));
		return;
	}
	if (strcmp(name, "BootStandard") == 0) {
		strcopy(LANGUAGE.BootStandard, val, sizeof(LANGUAGE.BootStandard));
		return;
	}
	if (strcmp(name, "Both") == 0) {
		strcopy(LANGUAGE.Both, val, sizeof(LANGUAGE.Both));
		return;
	}
	if (strcmp(name, "Cantcreatedirectory") == 0) {
		strcopy(LANGUAGE.Cantcreatedirectory, val, sizeof(LANGUAGE.Cantcreatedirectory));
		return;
	}
	if (strcmp(name, "Cancel") == 0) {
		strcopy(LANGUAGE.Cancel, val, sizeof(LANGUAGE.Cancel));
		return;
	}
	if (strcmp(name, "Cantbeformated") == 0) {
		strcopy(LANGUAGE.Cantbeformated, val, sizeof(LANGUAGE.Cantbeformated));
		return;
	}
	if (strcmp(name, "Cantdelete") == 0) {
		strcopy(LANGUAGE.Cantdelete, val, sizeof(LANGUAGE.Cantdelete));
		return;
	}
	if (strcmp(name, "ClicktoDownloadCovers") == 0) {
		strcopy(LANGUAGE.ClicktoDownloadCovers, val, sizeof(LANGUAGE.ClicktoDownloadCovers));
		return;
	}
	if (strcmp(name, "Clock") == 0) {
		strcopy(LANGUAGE.Clock, val, sizeof(LANGUAGE.Clock));
		return;
	}
	if (strcmp(name, "Close") == 0) {
		strcopy(LANGUAGE.Close, val, sizeof(LANGUAGE.Close));
		return;
	}
	if (strcmp(name, "Continueinstallgame") == 0) {
		strcopy(LANGUAGE.Continueinstallgame, val, sizeof(LANGUAGE.Continueinstallgame));
		return;
	}
	if (strcmp(name, "Console") == 0) {
		strcopy(LANGUAGE.Console, val, sizeof(LANGUAGE.Console));
		return;
	}
	if (strcmp(name, "ConsoleDefault") == 0) {
		strcopy(LANGUAGE.ConsoleDefault, val, sizeof(LANGUAGE.ConsoleDefault));
		return;
	}
	if (strcmp(name, "Consoleshouldbeunlockedtomodifyit") == 0) {
		strcopy(LANGUAGE.Consoleshouldbeunlockedtomodifyit, val, sizeof(LANGUAGE.Consoleshouldbeunlockedtomodifyit));
		return;
	}
	if (strcmp(name, "ConsoleLocked") == 0) {
		strcopy(LANGUAGE.ConsoleLocked, val, sizeof(LANGUAGE.ConsoleLocked));
		return;
	}
	if (strcmp(name, "Controllevel") == 0) {
		strcopy(LANGUAGE.Controllevel, val, sizeof(LANGUAGE.Controllevel));
		return;
	}
	if (strcmp(name, "CorrectPassword") == 0) {
		strcopy(LANGUAGE.CorrectPassword, val, sizeof(LANGUAGE.CorrectPassword));
		return;
	}
	if (strcmp(name, "Couldnotinitializenetwork") == 0) {
		strcopy(LANGUAGE.Couldnotinitializenetwork, val, sizeof(LANGUAGE.Couldnotinitializenetwork));
		return;
	}
	if (strcmp(name, "CouldnotopenDisc") == 0) {
		strcopy(LANGUAGE.CouldnotopenDisc, val, sizeof(LANGUAGE.CouldnotopenDisc));
		return;
	}
	if (strcmp(name, "CouldnotinitializeDIPmodule") == 0) {
		strcopy(LANGUAGE.CouldnotinitializeDIPmodule, val, sizeof(LANGUAGE.CouldnotinitializeDIPmodule));
		return;
	}
	if (strcmp(name, "count") == 0) {
		strcopy(LANGUAGE.count, val, sizeof(LANGUAGE.count));
		return;
	}
	if (strcmp(name, "CoverDownload") == 0) {
		strcopy(LANGUAGE.CoverDownload, val, sizeof(LANGUAGE.CoverDownload));
		return;
	}
	if (strcmp(name, "CoverPath") == 0) {
		strcopy(LANGUAGE.CoverPath, val, sizeof(LANGUAGE.CoverPath));
		return;
	}
	if (strcmp(name, "CoverpathChanged") == 0) {
		strcopy(LANGUAGE.CoverpathChanged, val, sizeof(LANGUAGE.CoverpathChanged));
		return;
	}
	if (strcmp(name, "Coverpathchange") == 0) {
		strcopy(LANGUAGE.Coverpathchange, val, sizeof(LANGUAGE.Coverpathchange));
		return;
	}
	if (strcmp(name, "Cheatcodespath") == 0) {
		strcopy(LANGUAGE.Cheatcodespath, val, sizeof(LANGUAGE.Cheatcodespath));
		return;
	}
	if (strcmp(name, "Cheatcodespathchanged") == 0) {
		strcopy(LANGUAGE.Cheatcodespathchanged, val, sizeof(LANGUAGE.Cheatcodespathchanged));
		return;
	}
	if (strcmp(name, "Credits") == 0) {
		strcopy(LANGUAGE.Credits, val, sizeof(LANGUAGE.Credits));
		return;
	}
	if (strcmp(name, "Custompaths") == 0) {
		strcopy(LANGUAGE.Custompaths, val, sizeof(LANGUAGE.Custompaths));
		return;
	}
	if (strcmp(name, "DiscImages") == 0) {
		strcopy(LANGUAGE.DiscImages, val, sizeof(LANGUAGE.DiscImages));
		return;
	}
	if (strcmp(name, "DiscimagePath") == 0) {
		strcopy(LANGUAGE.DiscimagePath, val, sizeof(LANGUAGE.DiscimagePath));
		return;
	}
	if (strcmp(name, "DiscpathChanged") == 0) {
		strcopy(LANGUAGE.DiscpathChanged, val, sizeof(LANGUAGE.DiscpathChanged));
		return;
	}
	if (strcmp(name, "Discpathchange") == 0) {
		strcopy(LANGUAGE.Discpathchange, val, sizeof(LANGUAGE.Discpathchange));
		return;
	}
	if (strcmp(name, "DiscDefault") == 0) {
		strcopy(LANGUAGE.DiscDefault, val, sizeof(LANGUAGE.DiscDefault));
		return;
	}
	if (strcmp(name, "Display") == 0) {
		strcopy(LANGUAGE.Display, val, sizeof(LANGUAGE.Display));
		return;
	}
	if (strcmp(name, "Doyouwanttoformat") == 0) {
		strcopy(LANGUAGE.Doyouwanttoformat, val, sizeof(LANGUAGE.Doyouwanttoformat));
		return;
	}
	if (strcmp(name, "Doyoureallywanttodelete") == 0) {
		strcopy(LANGUAGE.Doyoureallywanttodelete, val, sizeof(LANGUAGE.Doyoureallywanttodelete));
		return;
	}
	if (strcmp(name, "Doyouwanttoretryfor30secs") == 0) {
		strcopy(LANGUAGE.Doyouwanttoretryfor30secs, val, sizeof(LANGUAGE.Doyouwanttoretryfor30secs));
		return;
	}
	if (strcmp(name, "Doyouwanttochangelanguage") == 0) {
		strcopy(LANGUAGE.Doyouwanttochangelanguage, val, sizeof(LANGUAGE.Doyouwanttochangelanguage));
		return;
	}
	if (strcmp(name, "Downloadingfile") == 0) {
		strcopy(LANGUAGE.Downloadingfile, val, sizeof(LANGUAGE.Downloadingfile));
		return;
	}
	if (strcmp(name, "DownloadBoxartimage") == 0) {
		strcopy(LANGUAGE.DownloadBoxartimage, val, sizeof(LANGUAGE.DownloadBoxartimage));
		return;
	}
	if (strcmp(name, "Downloadfinished") == 0) {
		strcopy(LANGUAGE.Downloadfinished, val, sizeof(LANGUAGE.Downloadfinished));
		return;
	}
	if (strcmp(name, "Defaultgamesettings") == 0) {
		strcopy(LANGUAGE.Defaultgamesettings, val, sizeof(LANGUAGE.Defaultgamesettings));
		return;
	}
	if (strcmp(name, "Defaultsettings") == 0) {
		strcopy(LANGUAGE.Defaultsettings, val, sizeof(LANGUAGE.Defaultsettings));
		return;
	}
	if (strcmp(name, "Default") == 0) {
		strcopy(LANGUAGE.Default, val, sizeof(LANGUAGE.Default));
		return;
	}
	if (strcmp(name, "diversepatches") == 0) {
		strcopy(LANGUAGE.diversepatches, val, sizeof(LANGUAGE.diversepatches));
		return;
	}
	if (strcmp(name, "Error") == 0) {
		strcopy(LANGUAGE.Error, val, sizeof(LANGUAGE.Error));
		return;
	}
	if (strcmp(name, "Error002fix") == 0) {
		strcopy(LANGUAGE.Error002fix, val, sizeof(LANGUAGE.Error002fix));
		return;
	}
	if (strcmp(name, "BOOTERROR") == 0) {
		strcopy(LANGUAGE.BOOTERROR, val, sizeof(LANGUAGE.BOOTERROR));
		return;
	}
	if (strcmp(name, "ErrorreadingDisc") == 0) {
		strcopy(LANGUAGE.ErrorreadingDisc, val, sizeof(LANGUAGE.ErrorreadingDisc));
		return;
	}
	if (strcmp(name, "ExitUSBISOLoader") == 0) {
		strcopy(LANGUAGE.ExitUSBISOLoader, val, sizeof(LANGUAGE.ExitUSBISOLoader));
		return;
	}
	if (strcmp(name, "fave") == 0) {
		strcopy(LANGUAGE.fave, val, sizeof(LANGUAGE.fave));
		return;
	}
	if (strcmp(name, "InitializingNetwork") == 0) {
		strcopy(LANGUAGE.InitializingNetwork, val, sizeof(LANGUAGE.InitializingNetwork));
		return;
	}
	if (strcmp(name, "InsertDisk") == 0) {
		strcopy(LANGUAGE.InsertDisk, val, sizeof(LANGUAGE.InsertDisk));
		return;
	}
	if (strcmp(name, "InsertaWiiDisc") == 0) {
		strcopy(LANGUAGE.InsertaWiiDisc, val, sizeof(LANGUAGE.InsertaWiiDisc));
		return;
	}
	if (strcmp(name, "InsertaSDCardtodownloadimages") == 0) {
		strcopy(LANGUAGE.InsertaSDCardtodownloadimages, val, sizeof(LANGUAGE.InsertaSDCardtodownloadimages));
		return;
	}
	if (strcmp(name, "InsertaSDCardtousethatoption") == 0) {
		strcopy(LANGUAGE.InsertaSDCardtousethatoption, val, sizeof(LANGUAGE.InsertaSDCardtousethatoption));
		return;
	}
	if (strcmp(name, "InsertaSDCardtosave") == 0) {
		strcopy(LANGUAGE.InsertaSDCardtosave, val, sizeof(LANGUAGE.InsertaSDCardtosave));
		return;
	}
	if (strcmp(name, "InstallRenameandDeleteareunlocked") == 0) {
		strcopy(LANGUAGE.InstallRenameandDeleteareunlocked, val, sizeof(LANGUAGE.InstallRenameandDeleteareunlocked));
		return;
	}
	if (strcmp(name, "Installerror") == 0) {
		strcopy(LANGUAGE.Installerror, val, sizeof(LANGUAGE.Installerror));
		return;
	}
	if (strcmp(name, "Installagame") == 0) {
		strcopy(LANGUAGE.Installagame, val, sizeof(LANGUAGE.Installagame));
		return;
	}
	if (strcmp(name, "Installinggame") == 0) {
		strcopy(LANGUAGE.Installinggame, val, sizeof(LANGUAGE.Installinggame));
		return;
	}
	if (strcmp(name, "Failedtoboot") == 0) {
		strcopy(LANGUAGE.Failedtoboot, val, sizeof(LANGUAGE.Failedtoboot));
		return;
	}
	if (strcmp(name, "FailedtomountfrontSDcard") == 0) {
		strcopy(LANGUAGE.FailedtomountfrontSDcard, val, sizeof(LANGUAGE.FailedtomountfrontSDcard));
		return;
	}
	if (strcmp(name, "FailedtosetUSB") == 0) {
		strcopy(LANGUAGE.FailedtosetUSB, val, sizeof(LANGUAGE.FailedtosetUSB));
		return;
	}
	if (strcmp(name, "Failedformating") == 0) {
		strcopy(LANGUAGE.Failedformating, val, sizeof(LANGUAGE.Failedformating));
		return;
	}
	if (strcmp(name, "filesnotfoundontheserver") == 0) {
		strcopy(LANGUAGE.filesnotfoundontheserver, val, sizeof(LANGUAGE.filesnotfoundontheserver));
		return;
	}
	if (strcmp(name, "Filenotfound") == 0) {
		strcopy(LANGUAGE.Filenotfound, val, sizeof(LANGUAGE.Filenotfound));
		return;
	}
	if (strcmp(name, "filesleft") == 0) {
		strcopy(LANGUAGE.filesleft, val, sizeof(LANGUAGE.filesleft));
		return;
	}
	if (strcmp(name, "FlipX") == 0) {
		strcopy(LANGUAGE.FlipX, val, sizeof(LANGUAGE.FlipX));
		return;
	}
	if (strcmp(name, "For") == 0) {
		strcopy(LANGUAGE.For, val, sizeof(LANGUAGE.For));
		return;
	}
	if (strcmp(name, "Force") == 0) {
		strcopy(LANGUAGE.Force, val, sizeof(LANGUAGE.Force));
		return;
	}
	if (strcmp(name, "hour") == 0) {
		strcopy(LANGUAGE.hour, val, sizeof(LANGUAGE.hour));
		return;
	}
	if (strcmp(name, "Youneedtoformatapartition") == 0) {
		strcopy(LANGUAGE.Youneedtoformatapartition, val, sizeof(LANGUAGE.Youneedtoformatapartition));
		return;
	}
	if (strcmp(name, "Format") == 0) {
		strcopy(LANGUAGE.Format, val, sizeof(LANGUAGE.Format));
		return;
	}
	if (strcmp(name, "Formattingpleasewait") == 0) {
		strcopy(LANGUAGE.Formattingpleasewait, val, sizeof(LANGUAGE.Formattingpleasewait));
		return;
	}
	if (strcmp(name, "formated") == 0) {
		strcopy(LANGUAGE.formated, val, sizeof(LANGUAGE.formated));
		return;
	}
	if (strcmp(name, "Forhostingcovers") == 0) {
		strcopy(LANGUAGE.Forhostingcovers, val, sizeof(LANGUAGE.Forhostingcovers));
		return;
	}
	if (strcmp(name, "Forhostingupdatefiles") == 0) {
		strcopy(LANGUAGE.Forhostingupdatefiles, val, sizeof(LANGUAGE.Forhostingupdatefiles));
		return;
	}
	if (strcmp(name, "free") == 0) {
		strcopy(LANGUAGE.free, val, sizeof(LANGUAGE.free));
		return;
	}
	if (strcmp(name, "FreeSpace") == 0) {
		strcopy(LANGUAGE.FreeSpace, val, sizeof(LANGUAGE.FreeSpace));
		return;
	}
	if (strcmp(name, "FullShutdown") == 0) {
		strcopy(LANGUAGE.FullShutdown, val, sizeof(LANGUAGE.FullShutdown));
		return;
	}
	if (strcmp(name, "GameID") == 0) {
		strcopy(LANGUAGE.GameID, val, sizeof(LANGUAGE.GameID));
		return;
	}
	if (strcmp(name, "Games") == 0) {
		strcopy(LANGUAGE.Games, val, sizeof(LANGUAGE.Games));
		return;
	}
	if (strcmp(name, "Gameisalreadyinstalled") == 0) {
		strcopy(LANGUAGE.Gameisalreadyinstalled, val, sizeof(LANGUAGE.Gameisalreadyinstalled));
		return;
	}
	if (strcmp(name, "GameRegion") == 0) {
		strcopy(LANGUAGE.GameRegion, val, sizeof(LANGUAGE.GameRegion));
		return;
	}
	if (strcmp(name, "GameSize") == 0) {
		strcopy(LANGUAGE.GameSize, val, sizeof(LANGUAGE.GameSize));
		return;
	}
	if (strcmp(name, "GoBack") == 0) {
		strcopy(LANGUAGE.GoBack, val, sizeof(LANGUAGE.GoBack));
		return;
	}
	if (strcmp(name, "GUISettings") == 0) {
		strcopy(LANGUAGE.GUISettings, val, sizeof(LANGUAGE.GUISettings));
		return;
	}
	if (strcmp(name, "Gameload") == 0) {
		strcopy(LANGUAGE.Gameload, val, sizeof(LANGUAGE.Gameload));
		return;
	}
	if (strcmp(name, "GotoPage") == 0) {
		strcopy(LANGUAGE.GotoPage, val, sizeof(LANGUAGE.GotoPage));
		return;
	}
	if (strcmp(name, "Homemenu") == 0) {
		strcopy(LANGUAGE.Homemenu, val, sizeof(LANGUAGE.Homemenu));
		return;
	}
	if (strcmp(name, "HowtoShutdown") == 0) {
		strcopy(LANGUAGE.HowtoShutdown, val, sizeof(LANGUAGE.HowtoShutdown));
		return;
	}
	if (strcmp(name, "Keyboard") == 0) {
		strcopy(LANGUAGE.keyboard, val, sizeof(LANGUAGE.keyboard));
		return;
	}
	if (strcmp(name, "Language") == 0) {
		strcopy(LANGUAGE.Language, val, sizeof(LANGUAGE.Language));
		return;
	}
	if (strcmp(name, "Languagepathchanged") == 0) {
		strcopy(LANGUAGE.Languagepathchanged, val, sizeof(LANGUAGE.Languagepathchanged));
		return;
	}
	if (strcmp(name, "Langchange") == 0) {
		strcopy(LANGUAGE.Langchange, val, sizeof(LANGUAGE.Langchange));
		return;
	}
	if (strcmp(name, "Left") == 0) {
		strcopy(LANGUAGE.Left, val, sizeof(LANGUAGE.Left));
		return;
	}
	if (strcmp(name, "LikeSysMenu") == 0) {
		strcopy(LANGUAGE.LikeSysMenu, val, sizeof(LANGUAGE.LikeSysMenu));
		return;
	}
	if (strcmp(name, "ListSort") == 0) {
		strcopy(LANGUAGE.ListSort, val, sizeof(LANGUAGE.ListSort));
		return;
	}
	if (strcmp(name, "LoadingincIOS") == 0) {
		strcopy(LANGUAGE.LoadingincIOS, val, sizeof(LANGUAGE.LoadingincIOS));
		return;
	}
	if (strcmp(name, "Loadingstandardlanguage") == 0) {
		strcopy(LANGUAGE.Loadingstandardlanguage, val, sizeof(LANGUAGE.Loadingstandardlanguage));
		return;
	}
	if (strcmp(name, "Loadingstandardmusic") == 0) {
		strcopy(LANGUAGE.Loadingstandardmusic, val, sizeof(LANGUAGE.Loadingstandardmusic));
		return;
	}
	if (strcmp(name, "Locked") == 0) {
		strcopy(LANGUAGE.Locked, val, sizeof(LANGUAGE.Locked));
		return;
	}
	if (strcmp(name, "LockConsole") == 0) {
		strcopy(LANGUAGE.LockConsole, val, sizeof(LANGUAGE.LockConsole));
		return;
	}
	if (strcmp(name, "Patchcountrystrings") == 0) {
		strcopy(LANGUAGE.Patchcountrystrings, val, sizeof(LANGUAGE.Patchcountrystrings));
		return;
	}
	if (strcmp(name, "Missingfiles") == 0) {
		strcopy(LANGUAGE.Missingfiles, val, sizeof(LANGUAGE.Missingfiles));
		return;
	}
	if (strcmp(name, "Mature") == 0) {
		strcopy(LANGUAGE.Mature, val, sizeof(LANGUAGE.Mature));
		return;
	}
	if (strcmp(name, "Networkiniterror") == 0) {
		strcopy(LANGUAGE.Networkiniterror, val, sizeof(LANGUAGE.Networkiniterror));
		return;
	}
	if (strcmp(name, "Neither") == 0) {
		strcopy(LANGUAGE.Neither, val, sizeof(LANGUAGE.Neither));
		return;
	}
	if (strcmp(name, "Next") == 0) {
		strcopy(LANGUAGE.Next, val, sizeof(LANGUAGE.Next));
		return;
	}
	if (strcmp(name, "No") == 0) {
		strcopy(LANGUAGE.No, val, sizeof(LANGUAGE.No));
		return;
	}
	if (strcmp(name, "Nofilemissing") == 0) {
		strcopy(LANGUAGE.Nofilemissing, val, sizeof(LANGUAGE.Nofilemissing));
		return;
	}
	if (strcmp(name, "NoHDDfound") == 0) {
		strcopy(LANGUAGE.NoHDDfound, val, sizeof(LANGUAGE.NoHDDfound));
		return;
	}
	if (strcmp(name, "NoSDcardinserted") == 0) {
		strcopy(LANGUAGE.NoSDcardinserted, val, sizeof(LANGUAGE.NoSDcardinserted));
		return;
	}
	if (strcmp(name, "Nopartitionsfound") == 0) {
		strcopy(LANGUAGE.Nopartitionsfound, val, sizeof(LANGUAGE.Nopartitionsfound));
		return;
	}
	if (strcmp(name, "NoUSBDevice") == 0) {
		strcopy(LANGUAGE.NoUSBDevice, val, sizeof(LANGUAGE.NoUSBDevice));
		return;
	}
	if (strcmp(name, "NoWBFSpartitionfound") == 0) {
		strcopy(LANGUAGE.NoWBFSpartitionfound, val, sizeof(LANGUAGE.NoWBFSpartitionfound));
		return;
	}
	if (strcmp(name, "NormalCovers") == 0) {
		strcopy(LANGUAGE.NormalCovers, val, sizeof(LANGUAGE.NormalCovers));
		return;
	}
	if (strcmp(name, "Normal") == 0) {
		strcopy(LANGUAGE.Normal, val, sizeof(LANGUAGE.Normal));
		return;
	}
	if (strcmp(name, "NotaWiiDisc") == 0) {
		strcopy(LANGUAGE.NotaWiiDisc, val, sizeof(LANGUAGE.NotaWiiDisc));
		return;
	}
	if (strcmp(name, "NoUSBDevicefound") == 0) {
		strcopy(LANGUAGE.NoUSBDevicefound, val, sizeof(LANGUAGE.NoUSBDevicefound));
		return;
	}
	if (strcmp(name, "Notenoughfreespace") == 0) {
		strcopy(LANGUAGE.Notenoughfreespace, val, sizeof(LANGUAGE.Notenoughfreespace));
		return;
	}
	if (strcmp(name, "Notasupportedformat") == 0) {
		strcopy(LANGUAGE.Notasupportedformat, val, sizeof(LANGUAGE.Notasupportedformat));
		return;
	}
	if (strcmp(name, "notset") == 0) {
		strcopy(LANGUAGE.notset, val, sizeof(LANGUAGE.notset));
		return;
	}
	if (strcmp(name, "of") == 0) {
		strcopy(LANGUAGE.of, val, sizeof(LANGUAGE.of));
		return;
	}
	if (strcmp(name, "OFF") == 0) {
		strcopy(LANGUAGE.OFF, val, sizeof(LANGUAGE.OFF));
		return;
	}
	if (strcmp(name, "OfficialSite") == 0) {
		strcopy(LANGUAGE.OfficialSite, val, sizeof(LANGUAGE.OfficialSite));
		return;
	}
	if (strcmp(name, "OnlyInstall") == 0) {
		strcopy(LANGUAGE.OnlyInstall, val, sizeof(LANGUAGE.OnlyInstall));
		return;
	}
	if (strcmp(name, "ON") == 0) {
		strcopy(LANGUAGE.ON, val, sizeof(LANGUAGE.ON));
		return;
	}
	if (strcmp(name, "Parentalcontrol") == 0) {
		strcopy(LANGUAGE.Parentalcontrol, val, sizeof(LANGUAGE.Parentalcontrol));
		return;
	}
	if (strcmp(name, "Partition") == 0) {
		strcopy(LANGUAGE.Partition, val, sizeof(LANGUAGE.Partition));
		return;
	}
	if (strcmp(name, "Password") == 0) {
		strcopy(LANGUAGE.Password, val, sizeof(LANGUAGE.Password));
		return;
	}
	if (strcmp(name, "PasswordChanged") == 0) {
		strcopy(LANGUAGE.PasswordChanged, val, sizeof(LANGUAGE.PasswordChanged));
		return;
	}
	if (strcmp(name, "Passwordhasbeenchanged") == 0) {
		strcopy(LANGUAGE.Passwordhasbeenchanged, val, sizeof(LANGUAGE.Passwordhasbeenchanged));
		return;
	}
	if (strcmp(name, "Passwordchange") == 0) {
		strcopy(LANGUAGE.Passwordchange, val, sizeof(LANGUAGE.Passwordchange));
		return;
	}
	if (strcmp(name, "Plays") == 0) {
		strcopy(LANGUAGE.Plays, val, sizeof(LANGUAGE.Plays));
		return;
	}
	if (strcmp(name, "PowerofftheWii") == 0) {
		strcopy(LANGUAGE.PowerofftheWii, val, sizeof(LANGUAGE.PowerofftheWii));
		return;
	}
	if (strcmp(name, "Prev") == 0) {
		strcopy(LANGUAGE.Prev, val, sizeof(LANGUAGE.Prev));
		return;
	}
	if (strcmp(name, "PromptsButtons") == 0) {
		strcopy(LANGUAGE.PromptsButtons, val, sizeof(LANGUAGE.PromptsButtons));
		return;
	}
	if (strcmp(name, "ReloadSD") == 0) {
		strcopy(LANGUAGE.ReloadSD, val, sizeof(LANGUAGE.ReloadSD));
		return;
	}
	if (strcmp(name, "RenameGameonWBFS") == 0) {
		strcopy(LANGUAGE.RenameGameonWBFS, val, sizeof(LANGUAGE.RenameGameonWBFS));
		return;
	}
	if (strcmp(name, "Restart") == 0) {
		strcopy(LANGUAGE.Restart, val, sizeof(LANGUAGE.Restart));
		return;
	}
	if (strcmp(name, "Return") == 0) {
		strcopy(LANGUAGE.Return, val, sizeof(LANGUAGE.Return));
		return;
	}
	if (strcmp(name, "ReturntoWiiMenu") == 0) {
		strcopy(LANGUAGE.ReturntoWiiMenu, val, sizeof(LANGUAGE.ReturntoWiiMenu));
		return;
	}
	if (strcmp(name, "Right") == 0) {
		strcopy(LANGUAGE.Right, val, sizeof(LANGUAGE.Right));
		return;
	}
	if (strcmp(name, "Rumble") == 0) {
		strcopy(LANGUAGE.Rumble, val, sizeof(LANGUAGE.Rumble));
		return;
	}
	if (strcmp(name, "QuickBoot") == 0) {
		strcopy(LANGUAGE.QuickBoot, val, sizeof(LANGUAGE.QuickBoot));
		return;
	}
	if (strcmp(name, "Save") == 0) {
		strcopy(LANGUAGE.Save, val, sizeof(LANGUAGE.Save));
		return;
	}
	if (strcmp(name, "SaveFailed") == 0) {
		strcopy(LANGUAGE.SaveFailed, val, sizeof(LANGUAGE.SaveFailed));
		return;
	}
	if (strcmp(name, "theUSBLoaderandreleasingthesourcecodethe") == 0) {
		strcopy(LANGUAGE.theUSBLoaderandreleasingthesourcecode, val, sizeof(LANGUAGE.theUSBLoaderandreleasingthesourcecode));
		return;
	}
	if (strcmp(name, "secondsleft") == 0) {
		strcopy(LANGUAGE.secondsleft, val, sizeof(LANGUAGE.secondsleft));
		return;
	}
	if (strcmp(name, "SelectthePartition") == 0) {
		strcopy(LANGUAGE.SelectthePartition, val, sizeof(LANGUAGE.SelectthePartition));
		return;
	}
	if (strcmp(name, "youwanttoformat") == 0) {
		strcopy(LANGUAGE.youwanttoformat, val, sizeof(LANGUAGE.youwanttoformat));
		return;
	}
	if (strcmp(name, "settings") == 0) {
		strcopy(LANGUAGE.settings, val, sizeof(LANGUAGE.settings));
		return;
	}
	if (strcmp(name, "Setasbackgroundmusic") == 0) {
		strcopy(LANGUAGE.Setasbackgroundmusic, val, sizeof(LANGUAGE.Setasbackgroundmusic));
		return;
	}
	if (strcmp(name, "Sound") == 0) {
		strcopy(LANGUAGE.Sound, val, sizeof(LANGUAGE.Sound));
		return;
	}
	if (strcmp(name, "ShutdowntoIdle") == 0) {
		strcopy(LANGUAGE.ShutdowntoIdle, val, sizeof(LANGUAGE.ShutdowntoIdle));
		return;
	}
	if (strcmp(name, "ShutdownSystem") == 0) {
		strcopy(LANGUAGE.ShutdownSystem, val, sizeof(LANGUAGE.ShutdownSystem));
		return;
	}
	if (strcmp(name, "Standard") == 0) {
		strcopy(LANGUAGE.Standard, val, sizeof(LANGUAGE.Standard));
		return;
	}
	if (strcmp(name, "Success") == 0) {
		strcopy(LANGUAGE.Success, val, sizeof(LANGUAGE.Success));
		return;
	}
	if (strcmp(name, "Successfullyinstalled") == 0) {
		strcopy(LANGUAGE.Successfullyinstalled, val, sizeof(LANGUAGE.Successfullyinstalled));
		return;
	}
	if (strcmp(name, "Successfullydeleted") == 0) {
		strcopy(LANGUAGE.Successfullydeleted, val, sizeof(LANGUAGE.Successfullydeleted));
		return;
	}
	if (strcmp(name, "SuccessfullySaved") == 0) {
		strcopy(LANGUAGE.SuccessfullySaved, val, sizeof(LANGUAGE.SuccessfullySaved));
		return;
	}
	if (strcmp(name, "SystemDefault") == 0) {
		strcopy(LANGUAGE.SystemDefault, val, sizeof(LANGUAGE.SystemDefault));
		return;
	}
	if (strcmp(name, "Thanksto") == 0) {
		strcopy(LANGUAGE.Thanksto, val, sizeof(LANGUAGE.Thanksto));
		return;
	}
	if (strcmp(name, "ThemePath") == 0) {
		strcopy(LANGUAGE.ThemePath, val, sizeof(LANGUAGE.ThemePath));
		return;
	}
	if (strcmp(name, "ThemepathChanged") == 0) {
		strcopy(LANGUAGE.ThemepathChanged, val, sizeof(LANGUAGE.ThemepathChanged));
		return;
	}
	if (strcmp(name, "Themepathchange") == 0) {
		strcopy(LANGUAGE.Themepathchange, val, sizeof(LANGUAGE.Themepathchange));
		return;
	}
	if (strcmp(name, "Titlestxtpath") == 0) {
		strcopy(LANGUAGE.Titlestxtpath, val, sizeof(LANGUAGE.Titlestxtpath));
		return;
	}
	if (strcmp(name, "Titlestxtpathchange") == 0) {
		strcopy(LANGUAGE.Titlestxtpathchange, val, sizeof(LANGUAGE.Titlestxtpathchange));
		return;
	}
	if (strcmp(name, "TitlestxtpathChanged") == 0) {
		strcopy(LANGUAGE.TitlestxtpathChanged, val, sizeof(LANGUAGE.TitlestxtpathChanged));
		return;
	}
	if (strcmp(name, "Try") == 0) {
		strcopy(LANGUAGE.Try, val, sizeof(LANGUAGE.Try));
		return;
	}
	if (strcmp(name, "Tooltips") == 0) {
		strcopy(LANGUAGE.Tooltips, val, sizeof(LANGUAGE.Tooltips));
		return;
	}
	if (strcmp(name, "Timeleft") == 0) {
		strcopy(LANGUAGE.Timeleft, val, sizeof(LANGUAGE.Timeleft));
		return;
	}
	if (strcmp(name, "Unlocked") == 0) {
		strcopy(LANGUAGE.Unlocked, val, sizeof(LANGUAGE.Unlocked));
		return;
	}
	if (strcmp(name, "UnlockConsoletousethisOption") == 0) {
		strcopy(LANGUAGE.UnlockConsoletousethisOption, val, sizeof(LANGUAGE.UnlockConsoletousethisOption));
		return;
	}
	if (strcmp(name, "Unicodefix") == 0) {
		strcopy(LANGUAGE.Unicodefix, val, sizeof(LANGUAGE.Unicodefix));
		return;
	}
	if (strcmp(name, "Uninstall") == 0) {
		strcopy(LANGUAGE.Uninstall, val, sizeof(LANGUAGE.Uninstall));
		return;
	}
	if (strcmp(name, "Updatepath") == 0) {
		strcopy(LANGUAGE.Updatepath, val, sizeof(LANGUAGE.Updatepath));
		return;
	}
	if (strcmp(name, "Updatepathchanged") == 0) {
		strcopy(LANGUAGE.Updatepathchanged, val, sizeof(LANGUAGE.Updatepathchanged));
		return;
	}
	if (strcmp(name, "CheckingforUpdates") == 0) {
		strcopy(LANGUAGE.CheckingforUpdates, val, sizeof(LANGUAGE.CheckingforUpdates));
		return;
	}
	if (strcmp(name, "Updatefailed") == 0) {
		strcopy(LANGUAGE.Updatefailed, val, sizeof(LANGUAGE.Updatefailed));
		return;
	}
	if (strcmp(name, "Updatedol") == 0) {
		strcopy(LANGUAGE.Updatedol, val, sizeof(LANGUAGE.Updatedol));
		return;
	}
	if (strcmp(name, "Updateall") == 0) {
		strcopy(LANGUAGE.Updateall, val, sizeof(LANGUAGE.Updateall));
		return;
	}
	if (strcmp(name, "Updateto") == 0) {
		strcopy(LANGUAGE.Updateto, val, sizeof(LANGUAGE.Updateto));
		return;
	}
	if (strcmp(name, "Update") == 0) {
		strcopy(LANGUAGE.Update, val, sizeof(LANGUAGE.Update));
		return;
	}
	if (strcmp(name, "USBLoaderisprotected") == 0) {
		strcopy(LANGUAGE.USBLoaderisprotected, val, sizeof(LANGUAGE.USBLoaderisprotected));
		return;
	}
	if (strcmp(name, "USBDevicenotfound") == 0) {
		strcopy(LANGUAGE.USBDevicenotfound, val, sizeof(LANGUAGE.USBDevicenotfound));
		return;
	}
	if (strcmp(name, "VideoMode") == 0) {
		strcopy(LANGUAGE.VideoMode, val, sizeof(LANGUAGE.VideoMode));
		return;
	}
	if (strcmp(name, "VIDTVPatch") == 0) {
		strcopy(LANGUAGE.VIDTVPatch, val, sizeof(LANGUAGE.VIDTVPatch));
		return;
	}
	if (strcmp(name, "Volume") == 0) {
		strcopy(LANGUAGE.Volume, val, sizeof(LANGUAGE.Volume));
		return;
	}
	if (strcmp(name, "SFXVolume") == 0) {
		strcopy(LANGUAGE.SFXVolume, val, sizeof(LANGUAGE.SFXVolume));
		return;
	}
	if (strcmp(name, "Waiting") == 0) {
		strcopy(LANGUAGE.Waiting, val, sizeof(LANGUAGE.Waiting));
		return;
	}
	if (strcmp(name, "WaitingforUSBDevice") == 0) {
		strcopy(LANGUAGE.WaitingforUSBDevice, val, sizeof(LANGUAGE.WaitingforUSBDevice));
		return;
	}
	if (strcmp(name, "WidescreenFix") == 0) {
		strcopy(LANGUAGE.WidescreenFix, val, sizeof(LANGUAGE.WidescreenFix));
		return;
	}
	if (strcmp(name, "WiiMenu") == 0) {
		strcopy(LANGUAGE.WiiMenu, val, sizeof(LANGUAGE.WiiMenu));
		return;
	}
	if (strcmp(name, "Wiilight") == 0) {
		strcopy(LANGUAGE.Wiilight, val, sizeof(LANGUAGE.Wiilight));
		return;
	}
	if (strcmp(name, "WrongPassword") == 0) {
		strcopy(LANGUAGE.WrongPassword, val, sizeof(LANGUAGE.WrongPassword));
		return;
	}
	if (strcmp(name, "Yes") == 0) {
		strcopy(LANGUAGE.Yes, val, sizeof(LANGUAGE.Yes));
		return;
	}
	if (strcmp(name, "YoudonthavecIOS") == 0) {
		strcopy(LANGUAGE.YoudonthavecIOS, val, sizeof(LANGUAGE.YoudonthavecIOS));
		return;
	}
	if (strcmp(name, "Japanese") == 0) {
		strcopy(LANGUAGE.Japanese, val, sizeof(LANGUAGE.Japanese));
		return;
	}
	if (strcmp(name, "German") == 0) {
		strcopy(LANGUAGE.German, val, sizeof(LANGUAGE.German));
		return;
	}
	if (strcmp(name, "English") == 0) {
		strcopy(LANGUAGE.English, val, sizeof(LANGUAGE.English));
		return;
	}
	if (strcmp(name, "French") == 0) {
		strcopy(LANGUAGE.French, val, sizeof(LANGUAGE.French));
		return;
	}
	if (strcmp(name, "Spanish") == 0) {
		strcopy(LANGUAGE.Spanish, val, sizeof(LANGUAGE.Spanish));
		return;
	}
	if (strcmp(name, "Italian") == 0) {
		strcopy(LANGUAGE.Italian, val, sizeof(LANGUAGE.Italian));
		return;
	}
	if (strcmp(name, "Dutch") == 0) {
		strcopy(LANGUAGE.Dutch, val, sizeof(LANGUAGE.Dutch));
		return;
	}
	if (strcmp(name, "SChinese") == 0) {
		strcopy(LANGUAGE.SChinese, val, sizeof(LANGUAGE.SChinese));
		return;
	}
	if (strcmp(name, "TChinese") == 0) {
		strcopy(LANGUAGE.TChinese, val, sizeof(LANGUAGE.TChinese));
		return;
	}
	if (strcmp(name, "Korean") == 0) {
		strcopy(LANGUAGE.Korean, val, sizeof(LANGUAGE.Korean));
		return;
	}
	if (strcmp(name, "Successfullyupdated") == 0) {
		strcopy(LANGUAGE.Successfullyupdated, val, sizeof(LANGUAGE.Successfullyupdated));
		return;
	}
	if (strcmp(name, "Nonewupdates") == 0) {
		strcopy(LANGUAGE.Nonewupdates, val, sizeof(LANGUAGE.Nonewupdates));
		return;
	}
	if (strcmp(name, "Restarting") == 0) {
		strcopy(LANGUAGE.Restarting, val, sizeof(LANGUAGE.Restarting));
		return;
	}
	if (strcmp(name, "available") == 0) {
		strcopy(LANGUAGE.available, val, sizeof(LANGUAGE.available));
		return;
	}
	if (strcmp(name, "Doyouwanttoupdate") == 0) {
		strcopy(LANGUAGE.Doyouwanttoupdate, val, sizeof(LANGUAGE.Doyouwanttoupdate));
		return;
	}
	if (strcmp(name, "updating") == 0) {
		strcopy(LANGUAGE.updating, val, sizeof(LANGUAGE.updating));
		return;
	}
    if (strcmp(name, "XMLTitles") == 0) {
		strcopy(LANGUAGE.XMLTitles, val, sizeof(LANGUAGE.XMLTitles));
		return;
	}


}

