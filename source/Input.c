/****************************************************************************
 * Copyright (C) 2010-2012
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
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Input.h"

FILE * logFile = NULL;

char CheckInput()
{
    if(!kbhit())
        return 0;

    char inputChar = getch();

    if(inputChar == 'f')
    {
        if(logFile)
        {
            printf("\n\nStopped logging.\n\n");
            fclose(logFile);
            logFile = NULL;
        }
        else
        {
            logFile = fopen("GeckoLog.txt", "wb");
            if(logFile)
                printf("\n\nStarted logging to GeckoLog.txt.\n\n");
            else
                printf("\n\nCan't create GeckoLog.txt.\n\n");
        }
    }
    else if(inputChar == 'c')
    {
        system(CLEAR);
    }

    return inputChar;
}

#ifndef WIN32
#include <termios.h>

int kbhit(void)
{
    struct termios term, oterm;
    int fd = 0;
    int c = 0;

    tcgetattr(fd, &oterm);

    memcpy(&term, &oterm, sizeof(term));

    term.c_lflag = term.c_lflag & (!ICANON);

    term.c_cc[VMIN] = 0;

    term.c_cc[VTIME] = 1;

    tcsetattr(fd, TCSANOW, &term);

    c = getchar();

    tcsetattr(fd, TCSANOW, &oterm);

    if (c != -1)
        ungetc(c, stdin);

    return ((c != -1) ? 1 : 0);
}

int getch()
{
   static int ch = -1, fd = 0;

   struct termios neu, alt;

   fd = fileno(stdin);

   tcgetattr(fd, &alt);

   neu = alt;

   neu.c_lflag &= ~(ICANON|ECHO);

   tcsetattr(fd, TCSANOW, &neu);

   ch = getchar();

   tcsetattr(fd, TCSANOW, &alt);

   return ch;

}

#endif //WIN32
