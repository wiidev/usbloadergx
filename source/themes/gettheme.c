#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gctypes.h>
#include <ogc/gx.h>

enum
{
	ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER, ALIGN_TOP, ALIGN_BOTTOM, ALIGN_MIDDLE
};

typedef struct _MSG
{
	u32 id;
	char* msgstr;
	struct _MSG *next;
} MSG;
static MSG *baseMSG=0;


#define HASHWORDBITS 32

/* Defines the so called `hashpjw' function by P.J. Weinberger
   [see Aho/Sethi/Ullman, COMPILERS: Principles, Techniques and Tools,
   1986, 1987 Bell Telephone Laboratories, Inc.]  */
static inline u32 hash_string (const char *str_param)
{
	u32 hval, g;
	const char *str = str_param;

	/* Compute the hash value for the given string.  */
	hval = 0;
	while (*str != '\0')
	{
		hval <<= 4;
		hval += (u8) *str++;
		g = hval & ((u32) 0xf << (HASHWORDBITS - 4));
		if (g != 0)
		{
			hval ^= g >> (HASHWORDBITS - 8);
			hval ^= g;
		}
	}
	return hval;
}


static MSG *findMSG(u32 id)
{
	MSG *msg;
	for(msg=baseMSG; msg; msg=msg->next)
	{
		if(msg->id == id)
			return msg;
	}
	return NULL;
}

static MSG *setMSG(const char *msgid, const char *msgstr)
{
	u32 id = hash_string(msgid);
	MSG *msg = findMSG(id);
	if(!msg)
	{
		msg = (MSG *)malloc(sizeof(MSG));
		msg->id		= id;
		msg->msgstr = NULL;
		msg->next	= baseMSG;
		baseMSG		= msg;
	}
	if(msg)
	{
		if(msgstr)
		{
			if(msg->msgstr) free(msg->msgstr);
			msg->msgstr = strdup(msgstr);
		}
		return msg;
	}
	return NULL;
}

static inline void ClearPrefixes(char * msg)
{
	if(!msg)
		return;

	const char * ptr = msg;

	int i = 0;

	while(ptr[0] != '\0')
	{
		if(ptr[0] == '\\' && (ptr[1] == '\\' || ptr[1] == '"'))
		{
			++ptr;
		}

		msg[i] = ptr[0];

		++i;
		++ptr;
	}

	msg[i] = '\0';
}

void ThemeCleanUp(void)
{
	while(baseMSG)
	{
		MSG *nextMsg =baseMSG->next;
		free(baseMSG->msgstr);
		free(baseMSG);
		baseMSG = nextMsg;
	}
}

bool LoadTheme(const char* themeFile)
{
	FILE *f;
	char line[200];
	char *lastID=NULL;

	ThemeCleanUp();
	f = fopen(themeFile, "r");
	if(!f)
		return false;

	while (fgets(line, sizeof(line), f))
	{
		// lines starting with # are comments
		if (line[0] == '#')
			continue;
		else if (strncmp(line, "msgid \"", 7) == 0)
		{
			char *msgid, *end;
			if(lastID) { free(lastID); lastID=NULL;}
			msgid = &line[7];
			end = strrchr(msgid, '"');
			if(end && end-msgid>1)
			{
				*end = 0;
				ClearPrefixes(msgid);
				lastID = strdup(msgid);
			}
		}
		else if (strncmp(line, "msgstr \"", 8) == 0)
		{
			char *msgstr, *end;

			if(lastID == NULL)
				continue;

			msgstr = &line[8];
			end = strrchr(msgstr, '"');
			if(end && end-msgstr>1)
			{
				*end = 0;
				ClearPrefixes(msgstr);
				setMSG(lastID, msgstr);
			}
			free(lastID);
			lastID=NULL;
		}
	}

	fclose(f);
	return true;
}

int getThemeInt(const char *msgid)
{
	MSG *msg = findMSG(hash_string(msgid));
	if(msg) return atoi(msg->msgstr);
	return atoi(msgid);
}

float getThemeFloat(const char *msgid)
{
	MSG *msg = findMSG(hash_string(msgid));
	if(msg) return atof(msg->msgstr);
	return atof(msgid);
}

int getThemeAlignment(const char *msgid)
{
	MSG *msg = findMSG(hash_string(msgid));

	const char * string = msgid;
	if(msg)
		string = msg->msgstr;

	while(*string == ' ') string++;

	if(strncasecmp(string, "left", strlen("left")) == 0)
		return ALIGN_LEFT;

	else if(strncasecmp(string, "right", strlen("right")) == 0)
		return ALIGN_RIGHT;

	else if(strncasecmp(string, "center", strlen("center")) == 0)
		return ALIGN_CENTER;

	else if(strncasecmp(string, "top", strlen("top")) == 0)
		return ALIGN_TOP;

	else if(strncasecmp(string, "bottom", strlen("bottom")) == 0)
		return ALIGN_BOTTOM;

	else if(strncasecmp(string, "middle", strlen("middle")) == 0)
		return ALIGN_MIDDLE;

	return -1;
}

GXColor getThemeColor(const char *msgid)
{
	MSG *msg = findMSG(hash_string(msgid));

	const char * string = msgid;
	if(msg)
		string = msg->msgstr;

	GXColor color = (GXColor) {0, 0, 0, 0};

	while(*string == ' ') string++;

	while(*string != '\0')
	{
		if(*string == 'r')
		{
			string++;
			while(*string == ' ' || *string == '=' || *string == ',') string++;

			if(*string == '\0')
				break;

			color.r = atoi(string) & 0xFF;
		}
		else if(*string == 'g')
		{
			string++;
			while(*string == ' ' || *string == '=' || *string == ',') string++;

			if(*string == '\0')
				break;

			color.g = atoi(string) & 0xFF;
		}
		else if(*string == 'b')
		{
			string++;
			while(*string == ' ' || *string == '=' || *string == ',') string++;

			if(*string == '\0')
				break;

			color.b = atoi(string) & 0xFF;
		}
		else if(*string == 'a')
		{
			string++;
			while(*string == ' ' || *string == '=' || *string == ',') string++;

			if(*string == '\0')
				break;

			color.a = atoi(string) & 0xFF;
		}
		else if(*string == '-')
		{
			break;
		}

		++string;
	}

	return color;
}
