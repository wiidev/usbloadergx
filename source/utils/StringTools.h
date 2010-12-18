/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#ifndef __STRING_TOOLS_H
#define __STRING_TOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

//! fmt and wfmt can only be used once at a session and the strings needs
//! to be copied afterwards. A second use overwrites the first string.
const char * fmt(const char * format, ...);
const wchar_t * wfmt(const char * format, ...);
bool char2wchar_t(const char * src, wchar_t * dest);
int strtokcmp(const char * string, const char * compare, const char * separator);
const char * FullpathToFilename(const char *path);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif /* __STRING_TOOLS_H */

