#include <gccore.h>

const char* CONF_GetLanguageString(void)
{
	static int confLang = 0xdead;

	if(confLang == 0xdead)
		confLang = CONF_GetLanguage();

	const char*lang;
	switch( confLang )
	{
	case CONF_LANG_JAPANESE:		lang = "JPN"; break;
	default:
	case CONF_LANG_ENGLISH:			lang = "ENG"; break;
	case CONF_LANG_GERMAN:			lang = "GER"; break;
	case CONF_LANG_FRENCH:			lang = "FRA"; break;
	case CONF_LANG_SPANISH:			lang = "SPA"; break;
	case CONF_LANG_ITALIAN:			lang = "ITA"; break;
	case CONF_LANG_DUTCH:			lang = "NED"; break;
	case CONF_LANG_SIMP_CHINESE:
	case CONF_LANG_TRAD_CHINESE:	lang = "CHN"; break;
	case CONF_LANG_KOREAN:			lang = "KOR"; break;
	}
	return lang;
}
