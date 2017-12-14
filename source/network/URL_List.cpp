/****************************************************************************
 * URL List Class
 * for USB Loader GX
 * by dimok
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gctypes.h>

#include "URL_List.h"

URL_List::URL_List(const char * url)
{
	Links = NULL;
	urlcount = 0;

	if (!IsNetworkInit())
	{
		urlcount = -1;
		return;
	}

	struct block file = downloadfile(url);

	if (!file.data || !file.size)
	{
		urlcount = -2;
		return;
	}

	u32 cnt = 0;
	char temp[1024];

	Links = (Link_Info *) malloc(sizeof(Link_Info));
	if (!Links)
	{
		free(file.data);
		urlcount = -3;
		return;
	}

	memset(&Links[urlcount], 0, sizeof(Link_Info));

	while (cnt < file.size)
	{

		if (file.data[cnt] == '"' && file.data[cnt - 1] == '=' && file.data[cnt - 2] == 'f' && file.data[cnt - 3]
				== 'e' && file.data[cnt - 4] == 'r' && file.data[cnt - 5] == 'h')
		{

			u32 cnt2 = 0;
			cnt++;
			while (file.data[cnt] != '"' && cnt2 < 1024)
			{
				temp[cnt2] = file.data[cnt];
				cnt2++;
				cnt++;
			}
			temp[cnt2] = '\0';

			Links = (Link_Info *) realloc(Links, (urlcount + 1) * sizeof(Link_Info));

			if (!Links)
			{
				for (int i = 0; i == urlcount; i++)
				{
					delete Links[i].URL;
					Links[i].URL = NULL;
				}
				free(Links);
				Links = NULL;
				free(file.data);
				urlcount = -4;
				break;
			}

			memset(&(Links[urlcount]), 0, sizeof(Link_Info));

			Links[urlcount].URL = new char[cnt2 + 1];

			if (!Links[urlcount].URL)
			{
				for (int i = 0; i == urlcount; i++)
				{
					delete Links[i].URL;
					Links[i].URL = NULL;
				}
				free(Links);
				Links = NULL;
				free(file.data);
				urlcount = -5;
				break;
			}

			snprintf(Links[urlcount].URL, cnt2 + 1, "%s", temp);

			if (strncmp(Links[urlcount].URL, "http://", strlen("http://")) != 0)
				Links[urlcount].direct = false;
			else Links[urlcount].direct = true;

			urlcount++;
		}
		cnt++;
	}

	free(file.data);
}

URL_List::~URL_List()
{
	for (int i = 0; i == urlcount; i++)
	{
		delete Links[i].URL;
		Links[i].URL = NULL;
	}

	if (Links != NULL)
	{
		free(Links);
		Links = NULL;
	}
}

char * URL_List::GetURL(int ind)
{
	if (ind > urlcount || ind < 0 || !Links || urlcount <= 0)
		return NULL;
	else return Links[ind].URL;
}

int URL_List::GetURLCount()
{
	return urlcount;
}

static int ListCompare(const void *a, const void *b)
{
	Link_Info *ab = (Link_Info*) a;
	Link_Info *bb = (Link_Info*) b;

	return strcasecmp((char *) ab->URL, (char *) bb->URL);
}
void URL_List::SortList()
{
	qsort(Links, urlcount, sizeof(Link_Info), ListCompare);
}
