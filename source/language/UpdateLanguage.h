/****************************************************************************
 * language update
 * for USB Loader GX  *giantpune*
 ***************************************************************************/
#ifndef ___UPDATELANGUAGE_H_
#define ___UPDATELANGUAGE_H_

#define MAXLANGUAGEFILES	 50

//! Checks the language path for files ending in .lang and updates them (up to MAXLANGUAGEFILES)
//! This function expects that the network is already init before it is called

//! returns the number of files successfully updated
//! returns -2 if it can't find any .lang files in the path
//! return -1 if there is no network connection
int UpdateLanguageFiles();
int DownloadAllLanguageFiles();

#endif
