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
 *****************************************************d**********************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Input.h"
#include "network.h"

int main(int argc, char *argv[])
{
    char inputChar = 0;

    if(NetInit() < 0)
    {
        printf("Can't open socket.\n");
        usleep(999999);
        usleep(999999);
        return 0;
    }

    if(Bind() < 0)
    {
        printf("Can't bind socket.\n");
        usleep(999999);
        usleep(999999);
        return 0;
    }

    printf("WifiReader by Dimok\n");
    printf("q = quit\n");
    printf("f = start/stop writing log to file\n");
    printf("c = clear console\n");

	int ret;
    char data[RECEIVE_BLOCK_SIZE+1];
    memset(data, 0, sizeof(data));

    while(inputChar != 'q')
    {
        usleep(1000);

        inputChar = CheckInput();

        ret = NetRead(data, RECEIVE_BLOCK_SIZE);
        if(ret > 0)
        {
        	data[ret] = '\0';

            printf("%s", data);
            if(logFile)
                fwrite(data, 1, ret, logFile);
        }
    }

    if(logFile)
        fclose(logFile);

    CloseSocket();

    return 0;
}
