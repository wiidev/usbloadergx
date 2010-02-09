/****************************************************************************
 * Theme_List Class
 * for USB Loader GX
 * by dimok
 ***************************************************************************/
#ifndef ___THEMELIST_H_
#define ___THEMELIST_H_

#include "network/networkops.h"
#include "network/http.h"

typedef struct _theme_info
{
    char    *themetitle;
    char    *author;
    char    *imagelink;
    char    *downloadlink;
    u8      rating;
} Theme_Info;


class Theme_List
{
    public:
        //!Constructor
        //!\param url from where to get the list of links
        Theme_List(const char *url);
        //!Destructor
        ~Theme_List();
        //!Get Themes into a struct from the XML file amount
        bool ParseXML(const u8 * xmlfile);
        //!Get Theme amount
        int CountThemes(const u8 * xmlfile);
        //! Get the a theme title
        //!\param list index
        const char * GetThemeTitle(int index);
        //! Get the author of the theme
        //!\param list index
        const char * GetThemeAuthor(int index);
        //! Get the author of the theme
        //!\param list index
        const char * GetImageLink(int index);
        //! Get the download link of the theme
        //!\param list index
        const char * GetDownloadLink(int index);
        //! Get the number of links counted
        int GetThemeCount();
        //! Sort list
        void SortList();
    protected:
        int themescount;
        Theme_Info *Theme;
};

#endif
