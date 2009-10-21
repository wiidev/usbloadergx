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
    char    themetitle[100];
    char    author[50];
    char    *imagelink;
    char    *downloadlink;
    bool    direct[2];
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
        //! Is it a direct URL or just a file or path under the main url
        bool IsDirectImageLink(int index);
        //! Is it a direct URL or just a file or path under the main url
        bool IsDirectDownloadLink(int index);
        //! Get the number of links counted
        int GetThemeCount();
        //! Get the number of pages counted on which there are Themes
        int GetSitepageCount();
        //! Sort list
        void SortList();
    protected:
        int themescount;
        int sitepages;
        Theme_Info *Theme;
};

#endif
