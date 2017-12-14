/****************************************************************************
 * Network Operations
 * for USB Loader GX
 *
 * HTTP operations
 * Written by dhewg/bushing modified by dimok
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ogcsys.h>
#include <ogc/machine/processor.h>
#include <fcntl.h>

#include "prompts/PromptWindows.h"
#include "language/gettext.h"
#include "settings/CSettings.h"
#include "utils/timer.h"
#include "networkops.h"
#include "main.h"
#include "http.h"
#include "svnrev.h"
#include "gecko.h"
#include "update.h"

#define PORT			4299

/*** Incomming filesize ***/
u32 infilesize = 0;
u32 uncfilesize = 0;

bool updateavailable = false;
s32 connection;
static s32 socket;
static bool updatechecked = false;
static bool networkinitialized = false;
static bool checkincomming = false;
static bool waitforanswer = false;
static char IP[16];
char incommingIP[50];
char wiiloadVersion[2];

static lwp_t networkthread = LWP_THREAD_NULL;
static bool networkHalt = true;

/****************************************************************************
 * Initialize_Network
 ***************************************************************************/
void Initialize_Network(int retries)
{

	if (networkinitialized) return;

	s32 result;

	result = if_config(IP, NULL, NULL, true, retries);

	if (result < 0)
	{
		networkinitialized = false;
		return;
	}
	else
	{
		networkinitialized = true;
		return;
	}
}

/****************************************************************************
 * DeinitNetwork
 ***************************************************************************/
void DeinitNetwork(void)
{
	net_wc24cleanup();
	net_deinit();
	networkinitialized = false;
}

/****************************************************************************
 * Check if network was initialised
 ***************************************************************************/
bool IsNetworkInit(void)
{
	return networkinitialized;
}

/****************************************************************************
 * Get network IP
 ***************************************************************************/
char * GetNetworkIP(void)
{
	return IP;
}

/****************************************************************************
 * Get incomming IP
 ***************************************************************************/
char * GetIncommingIP(void)
{
	return incommingIP;
}

s32 network_request(s32 connect, const char * request, char * filename)
{
	if(connect == NET_DEFAULT_SOCK)
		connect = connection;

	char buf[1024];
	char *ptr = NULL;

	u32 cnt, size;
	s32 ret;

	/* Send request */
	ret = net_send(connect, request, strlen(request), 0);
	if (ret < 0) return ret;

	/* Clear buffer */
	memset(buf, 0, sizeof(buf));

	/* Read HTTP header */
	for (cnt = 0; !strstr(buf, "\r\n\r\n"); cnt++)
		if (net_recv(connect, buf + cnt, 1, 0) <= 0) return -1;

	/* HTTP request OK? */
	if (!strstr(buf, "HTTP/1.1 200 OK")) return -1;

	if (filename)
	{
		/* Get filename */
		ptr = strstr(buf, "filename=\"");

		if (ptr)
		{
			ptr += sizeof("filename=\"") - 1;

			for (cnt = 0; ptr[cnt] != '\r' && ptr[cnt] != '\n' && ptr[cnt] != '"'; cnt++)
			{
				filename[cnt] = ptr[cnt];
				filename[cnt + 1] = '\0';
			}
		}
	}

	/* Retrieve content size */
	ptr = strstr(buf, "Content-Length:");
	if (!ptr) return NET_SIZE_UNKNOWN;

	sscanf(ptr, "Content-Length: %lu", &size);
	return size;
}

s32 network_read(s32 connect, u8 *buf, u32 len)
{
	if(connect == NET_DEFAULT_SOCK)
		connect = connection;

	u32 read = 0;
	s32 ret = -1;

	/* Data to be read */
	while (read < len)
	{
		/* Read network data */
		ret = net_read(connect, buf + read, len - read);
		if (ret < 0) return ret;

		/* Read finished */
		if (!ret) break;

		/* Increment read variable */
		read += ret;
	}

	return read;
}

/****************************************************************************
 * Download request
 ***************************************************************************/
s32 download_request(const char * url, char * filename)
{
	//Check if the url starts with "http://", if not it is not considered a valid url
	if (strncmp(url, "http://", strlen("http://")) != 0)
	{
		return -1;
	}

	//Locate the path part of the url by searching for '/' past "http://"
	char *path = strchr(url + strlen("http://"), '/');

	//At the very least the url has to end with '/', ending with just a domain is invalid
	if (path == NULL)
	{
		return -1;
	}

	//Extract the domain part out of the url
	int domainlength = path - url - strlen("http://");

	if (domainlength == 0)
	{
		return -1;
	}

	char domain[domainlength + 1];
	strlcpy(domain, url + strlen("http://"), domainlength + 1);

	connection = GetConnection(domain);
	if (connection < 0)
	{
		return -1;
	}

	//Form a nice request header to send to the webserver
	char header[strlen(path) + strlen(domain) + strlen(url) + 100];
	sprintf(header, "GET %s HTTP/1.1\r\nHost: %s\r\nReferer: %s\r\nUser-Agent: USBLoaderGX\r\nConnection: close\r\n\r\n", path, domain, url);

	s32 filesize = network_request(connection, header, filename);

	return filesize;
}

/****************************************************************************
 * HTML HEAD request
 ***************************************************************************/
char * HEAD_Request(const char * url)
{
	if(strncmp(url, "http://", strlen("http://")) != 0)
	{
		gprintf("Not a valid URL");
		return NULL;
	}
	char *path = strchr(url + strlen("http://"), '/');

	if(!path)
	{
		gprintf("Not a valid URL path");
		return NULL;
	}

	int domainlength = path - url - strlen("http://");

	if(domainlength == 0)
	{
		gprintf("Not a valid domain");
		return NULL;
	}

	char domain[domainlength + 1];
	strncpy(domain, url + strlen("http://"), domainlength);
	domain[domainlength] = '\0';

	connection = GetConnection(domain);
	if(connection < 0)
	{
		gprintf("Could not connect to the server.");
		return NULL;
	}

	char header[strlen(path)+strlen(domain)*2+150];
	sprintf(header, "HEAD %s HTTP/1.1\r\nHost: %s\r\nReferer: %s\r\nUser-Agent: USB Loader GX\r\nConnection: close\r\n\r\n", path, domain, domain);

	/* Send request */
	s32 ret = net_send(connection, header, strlen(header), 0);
	if (ret < 0)
	{
		CloseConnection();
		return NULL;
	}

	u32 cnt = 0;
	char * buf = (char *) malloc(1024);
	memset(buf, 0, 1024);

	for (cnt = 0; !strstr(buf, "\r\n\r\n") && cnt < 1024; cnt++)
	{
		if(net_recv(connection, buf + cnt, 1, 0) <= 0)
		{
			CloseConnection();
			free(buf);
			return NULL;
		}
	}

	CloseConnection();

	return buf;
}

void CloseConnection()
{

	net_close(connection);

	if (waitforanswer)
	{
		net_close(socket);
		waitforanswer = false;
	}
}

/****************************************************************************
 * Test if connection to the address is available (PING)
 ***************************************************************************/
bool CheckConnection(const char *url, float timeout)
{
	//Check if the url starts with "http://", if not it is not considered a valid url
	if (strncmp(url, "http://", strlen("http://")) != 0)
		return false;

	//Locate the path part of the url by searching for '/' past "http://"
	char *path = strchr(url + strlen("http://"), '/');

	//At the very least the url has to end with '/', ending with just a domain is invalid
	if (path == NULL)
		return false;

	//Extract the domain part out of the url
	int domainlength = path - url - strlen("http://");
	if (domainlength == 0)
		return false;

	char domain[domainlength + 1];
	strlcpy(domain, url + strlen("http://"), domainlength + 1);

	//Parsing of the URL is done, start making an actual connection
	u32 ipaddress = getipbynamecached(domain);
	if (ipaddress == 0)
		return false;

	//Initialize socket
	s32 connection = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (connection < 0) return connection;

	s32 flags = net_fcntl(connection, F_GETFL, 0);
	if (flags >= 0) flags = net_fcntl(connection, F_SETFL, flags | 4);

	struct sockaddr_in connect_addr;
	memset(&connect_addr, 0, sizeof(connect_addr));
	connect_addr.sin_family = AF_INET;
	connect_addr.sin_port = 80;
	connect_addr.sin_addr.s_addr = getipbynamecached(domain);

	Timer netTime;

	int res = -1;
	while(res < 0 && res != -127 && netTime.elapsed() < timeout)
	{
		res = net_connect(connection, (struct sockaddr*) &connect_addr, sizeof(connect_addr));
		usleep(1000);
	}

	net_close(connection);

	return !(res < 0 && res != -127);
}


/****************************************************************************
 * Download request
 ***************************************************************************/
s32 DownloadWithResponse(const char * url, u8 **outbuffer, u32 *outsize)
{
	//Check if the url starts with "http://", if not it is not considered a valid url
	if (strncmp(url, "http://", strlen("http://")) != 0)
		return -1;

	//Locate the path part of the url by searching for '/' past "http://"
	char *path = strchr(url + strlen("http://"), '/');

	//At the very least the url has to end with '/', ending with just a domain is invalid
	if (path == NULL)
		return -1;

	//Extract the domain part out of the url
	int domainlength = path - url - strlen("http://");

	if (domainlength == 0)
		return -1;

	char domain[domainlength + 1];
	strlcpy(domain, url + strlen("http://"), domainlength + 1);

	int connect = GetConnection(domain);
	if (connect < 0)
		return -1;

	//Form a nice request header to send to the webserver
	char header[strlen(path) + strlen(domain) + strlen(url) + 100];
	sprintf(header, "GET %s HTTP/1.1\r\nHost: %s\r\nReferer: %s\r\nUser-Agent: USBLoaderGX\r\nConnection: close\r\n\r\n", path, domain, url);

	int ret = net_send(connect, header, strlen(header), 0);
	if(ret < 0)
	{
		net_close(connect);
		return ret;
	}

	int blocksize = 4096;

	u8 *buffer = (u8 *) malloc(blocksize);
	if(!buffer)
	{
		net_close(connect);
		return -1;
	}

	int done = 0;

	while(1)
	{
		int ret = network_read(connect, buffer+done, blocksize);

		if(ret < 0)
		{
			free(buffer);
			net_close(connect);
			return -1;
		}
		else if(ret == 0)
			break;

		done += ret;
		u8 *tmp = (u8 *) realloc(buffer, done+blocksize);
		if(!tmp)
		{
			free(buffer);
			net_close(connect);
			return -1;
		}

		buffer = tmp;
	}

	net_close(connect);

	u8 *tmp = (u8 *) realloc(buffer, done+1);
	if(!tmp)
	{
		free(buffer);
		return -1;
	}

	buffer = tmp;
	buffer[done] = 0;

	*outbuffer = buffer;
	*outsize = done;

	return done;
}


/****************************************************************************
 * NetworkWait
 ***************************************************************************/
int NetworkWait()
{
	if (!checkincomming) return -3;

	struct sockaddr_in sin;
	struct sockaddr_in client_address;
	socklen_t addrlen = sizeof(client_address);

	//Open socket
	socket = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

	if (socket == INVALID_SOCKET)
	{
		return socket;
	}

	sin.sin_family = AF_INET;
	sin.sin_port = htons( PORT );
	sin.sin_addr.s_addr = htonl( INADDR_ANY );

	if (net_bind(socket, (struct sockaddr*) &sin, sizeof(sin)) < 0)
	{
		net_close(socket);
		return -1;
	}

	if (net_listen(socket, 3) < 0)
	{
		net_close(socket);
		return -1;
	}

	connection = net_accept(socket, (struct sockaddr*) &client_address, &addrlen);

	sprintf(incommingIP, "%s", inet_ntoa(client_address.sin_addr));

	if (connection < 0)
	{
		net_close(connection);
		net_close(socket);
		return -4;

	}
	else
	{

		unsigned char haxx[9];
		//skip haxx
		net_read(connection, &haxx, 8);
		wiiloadVersion[0] = haxx[4];
		wiiloadVersion[1] = haxx[5];

		net_read(connection, &infilesize, 4);

		if (haxx[4] > 0 || haxx[5] > 4)
		{
			net_read(connection, &uncfilesize, 4); // Compressed protocol, read another 4 bytes
		}
		waitforanswer = true;
		checkincomming = false;
		networkHalt = true;
	}

	return 1;
}

/****************************************************************************
 * HaltNetwork
 ***************************************************************************/
void HaltNetworkThread()
{
	networkHalt = true;
	checkincomming = false;

	if (waitforanswer) CloseConnection();

	// wait for thread to finish
	while (!LWP_ThreadIsSuspended(networkthread))
		usleep(100);
}

/****************************************************************************
 * ResumeNetworkThread
 ***************************************************************************/
void ResumeNetworkThread()
{
	networkHalt = false;
	LWP_ResumeThread(networkthread);
}

/****************************************************************************
 * Resume NetworkWait
 ***************************************************************************/
void ResumeNetworkWait()
{
	networkHalt = true;
	checkincomming = true;
	waitforanswer = true;
	infilesize = 0;
	connection = -1;
	LWP_ResumeThread(networkthread);
}

/*********************************************************************************
 * Networkthread for background network initialize and update check with idle prio
 *********************************************************************************/
static void * networkinitcallback(void *arg)
{
	while (1)
	{
		if (!checkincomming && networkHalt)
			LWP_SuspendThread(networkthread);

		Initialize_Network();

		if (networkinitialized == true && updatechecked == false)
		{

			if (CheckUpdate() > 0) updateavailable = true;

			//suspend thread
			updatechecked = true;
			networkHalt = true;
		}

		if (checkincomming) NetworkWait();

		usleep(100000);
	}
	return NULL;
}

/****************************************************************************
 * InitNetworkThread with priority 0 (idle)
 ***************************************************************************/
void InitNetworkThread()
{
	LWP_CreateThread(&networkthread, networkinitcallback, NULL, NULL, 16384, 0);
}

/****************************************************************************
 * ShutdownThread
 ***************************************************************************/
void ShutdownNetworkThread()
{
	LWP_JoinThread(networkthread, NULL);
	networkthread = LWP_THREAD_NULL;
}
