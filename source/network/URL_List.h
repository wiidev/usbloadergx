/****************************************************************************
 * URL List Class
 * for USB Loader GX
 * by dimok
 ***************************************************************************/
#ifndef ___URLLIST_H_
#define ___URLLIST_H_

#include "network/networkops.h"
#include "network/http.h"

typedef struct
{
		char *URL;
		bool direct;
} Link_Info;

class URL_List
{
	public:
		//!Constructor
		//!\param url from where to get the list of links
		URL_List(const char *url);
		//!Destructor
		~URL_List();
		//! Get the a filepath of the list
		//!\param list index
		char * GetURL(int index);
		//! Is it a direct URL or just a file or path under the main url
		bool IsDirectURL(int index);
		//! Get the number of links counted
		int GetURLCount();
		//! Sort list
		void SortList();
	protected:
		int urlcount;
		Link_Info *Links;
};

#endif
