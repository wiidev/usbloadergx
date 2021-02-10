/****************************************************************************
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
#include <errno.h>
#include <network.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DESTINATION_IP "192.168.1.255"
#define DESTINATION_PORT 4405

#define GPRINTF_SIZE 256
#define WIFIGECKO_SIZE 1024

char wifigeckobuffer[WIFIGECKO_SIZE];

static int connection = -1;

void WifiGecko_Close()
{
	if (connection >= 0)
		net_close(connection);

	connection = -1;
}

int WifiGecko_Connect()
{
	if (connection >= 0)
		return connection;

	connection = net_socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (connection < 0)
		return connection;

	struct sockaddr_in connect_addr;
	memset(&connect_addr, 0, sizeof(connect_addr));
	connect_addr.sin_family = AF_INET;
	connect_addr.sin_port = htons(DESTINATION_PORT);
	inet_aton(DESTINATION_IP, &connect_addr.sin_addr);

	if (net_connect(connection, (struct sockaddr *)&connect_addr, sizeof(connect_addr)) < 0)
	{
		WifiGecko_Close();
		return -1;
	}

	return connection;
}

int WifiGecko_Send(const char *data, int datasize)
{
	if ((strlen(wifigeckobuffer) + datasize) < WIFIGECKO_SIZE)
		strcat(wifigeckobuffer, data);

	if (WifiGecko_Connect() < 0)
		return connection;

	u32 sendsize = strlen(wifigeckobuffer);

	while (net_get_status() == -EBUSY)
		usleep(10000);
	int ret = net_send(connection, wifigeckobuffer, sendsize, 0);
	if (ret < 0)
		WifiGecko_Close();

	memset(wifigeckobuffer, 0, WIFIGECKO_SIZE);
	return ret;
}

void wifi_printf(const char *format, ...)
{
	char gprintfBuffer[GPRINTF_SIZE];
	va_list va;
	va_start(va, format);
	size_t len = vsnprintf(gprintfBuffer, GPRINTF_SIZE - 1, format, va);
	va_end(va);
	if (len)
		WifiGecko_Send(gprintfBuffer, len);
}
