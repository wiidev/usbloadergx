/****************************************************************************
 * Theme_List Class
 * for USB Loader GX
 * by dimok
 ***************************************************************************/
#ifndef ___THEMELIST_H_
#define ___THEMELIST_H_

#include <vector>
#include <string>
#include "network/networkops.h"
#include "network/http.h"

typedef struct _theme_info
{
		std::string themetitle;
		std::string author;
		std::string imagelink;
		std::string downloadlink;
		u8 rating;
} Theme_Info;

class Theme_List
{
	public:
		//!Constructor
		//!\param url from where to get the list of links
		Theme_List(const char *url);
		//!Destructor
		~Theme_List();
		//! Get the a theme title
		//!\param list index
		const char * GetThemeTitle(int index) const;
		//! Get the author of the theme
		//!\param list index
		const char * GetThemeAuthor(int index) const;
		//! Get the author of the theme
		//!\param list index
		const char * GetImageLink(int index) const;
		//! Get the download link of the theme
		//!\param list index
		const char * GetDownloadLink(int index) const;
		//! Get the number of links counted
		int GetThemeCount() const { return ThemesList.size(); };
	protected:
		//!Get Themes into a struct from the XML file amount
		bool ParseXML(const char * xmlfile);
		std::vector<Theme_Info> ThemesList;
};

#endif
