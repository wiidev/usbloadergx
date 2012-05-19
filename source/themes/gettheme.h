#ifndef GETTHEME_H_
#define GETTHEME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <ogc/gx.h>
#include <gctypes.h>

int getThemeInt(const char *msgid);
float getThemeFloat(const char *msgid);
int getThemeAlignment(const char *msgid);
GXColor getThemeColor(const char *msgid);
bool LoadTheme(const char* themeFile);
void ThemeCleanUp(void);

#define thInt(s) getThemeInt(s)
#define thFloat(s) getThemeFloat(s)
#define thAlign(s) getThemeAlignment(s)
#define thColor(s) getThemeColor(s)

#ifdef __cplusplus
}
#endif

#endif
