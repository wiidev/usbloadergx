#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gctypes.h>
#include "gettext.h"

typedef struct _MSG
{
	u32 id;
	char* msgstr;
	struct _MSG *next;
} MSG;
static MSG *baseMSG = 0;

#define HASHWORDBITS 32

/* Defines the so called `hashpjw' function by P.J. Weinberger
 [see Aho/Sethi/Ullman, COMPILERS: Principles, Techniques and Tools,
 1986, 1987 Bell Telephone Laboratories, Inc.]  */
static inline u32 hash_string(const char *str_param)
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

/* Expand some escape sequences found in the argument string.  */
static char *
expand_escape(const char *str)
{
	char *retval, *rp;
	const char *cp = str;

	retval = (char *) malloc(strlen(str) + 1);
	if (retval == NULL) return NULL;
	rp = retval;

	while (cp[0] != '\0' && cp[0] != '\\')
		*rp++ = *cp++;
	if (cp[0] == '\0') goto terminate;
	do
	{

		/* Here cp[0] == '\\'.  */
		switch (*++cp)
		{
			case '\"': /* " */
				*rp++ = '\"';
				++cp;
				break;
			case 'a': /* alert */
				*rp++ = '\a';
				++cp;
				break;
			case 'b': /* backspace */
				*rp++ = '\b';
				++cp;
				break;
			case 'f': /* form feed */
				*rp++ = '\f';
				++cp;
				break;
			case 'n': /* new line */
				*rp++ = '\n';
				++cp;
				break;
			case 'r': /* carriage return */
				*rp++ = '\r';
				++cp;
				break;
			case 't': /* horizontal tab */
				*rp++ = '\t';
				++cp;
				break;
			case 'v': /* vertical tab */
				*rp++ = '\v';
				++cp;
				break;
			case '\\':
				*rp = '\\';
				++cp;
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			{
				int ch = *cp++ - '0';

				if (*cp >= '0' && *cp <= '7')
				{
					ch *= 8;
					ch += *cp++ - '0';

					if (*cp >= '0' && *cp <= '7')
					{
						ch *= 8;
						ch += *cp++ - '0';
					}
				}
				*rp = ch;
			}
				break;
			default:
				*rp = '\\';
				break;
		}

		while (cp[0] != '\0' && cp[0] != '\\')
			*rp++ = *cp++;
	} while (cp[0] != '\0');

	/* Terminate string.  */
	terminate: *rp = '\0';
	return retval;
}

static MSG *findMSG(u32 id)
{
	MSG *msg;
	for (msg = baseMSG; msg; msg = msg->next)
	{
		if (msg->id == id) return msg;
	}
	return NULL;
}

static MSG *setMSG(const char *msgid, const char *msgstr)
{
	u32 id = hash_string(msgid);
	MSG *msg = findMSG(id);
	if (!msg)
	{
		msg = (MSG *) malloc(sizeof(MSG));
		msg->id = id;
		msg->msgstr = NULL;
		msg->next = baseMSG;
		baseMSG = msg;
	}
	if (msg)
	{
		if (msgstr)
		{
			if (msg->msgstr) free(msg->msgstr);
			//msg->msgstr = strdup(msgstr);
			msg->msgstr = expand_escape(msgstr);
		}
		return msg;
	}
	return NULL;
}
void gettextCleanUp(void)
{
	while (baseMSG)
	{
		MSG *nextMsg = baseMSG->next;
		free(baseMSG->msgstr);
		free(baseMSG);
		baseMSG = nextMsg;
	}
}

bool gettextLoadLanguage(const char* langFile)
{
	FILE *f;
	char line[512];
	char *lastID = NULL;

	gettextCleanUp();
	f = fopen(langFile, "r");
	if (!f) return false;

	while (fgets(line, sizeof(line), f))
	{
		// lines starting with # are comments
		if (line[0] == '#')
			continue;
		else if (strncmp(line, "msgid \"", 7) == 0)
		{
			char *msgid, *end;
			if (lastID)
			{
				free(lastID);
				lastID = NULL;
			}
			msgid = &line[7];
			end = strrchr(msgid, '"');
			if (end && end - msgid > 1)
			{
				*end = 0;
				lastID = strdup(msgid);
			}
		}
		else if (strncmp(line, "msgstr \"", 8) == 0)
		{
			char *msgstr, *end;

			if (lastID == NULL) continue;

			msgstr = &line[8];
			end = strrchr(msgstr, '"');
			if (end && end - msgstr > 1)
			{
				*end = 0;
				setMSG(lastID, msgstr);
			}
			free(lastID);
			lastID = NULL;
		}

	}

	fclose(f);
	return true;
}
const char *gettext(const char *msgid)
{
	if(!msgid[0]) return NULL;
	MSG *msg = findMSG(hash_string(msgid));
	if (msg && msg->msgstr) return msg->msgstr;
	return msgid;
}

