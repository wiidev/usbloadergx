/****************************************************************************
 * Updater for USB Loader GX
 *
 * HTTP operations
 * Written by dhewg/bushing modified by dimok
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ogcsys.h>
#include <ogc/machine/processor.h>

#include "prompts/PromptWindows.h"
#include "settings/cfg.h"
#include "main.h"
#include "http.h"

static s32 connection;
static bool updatechecked = false;
static bool networkinitialized = false;
static char IP[16];

static lwp_t networkthread = LWP_THREAD_NULL;
static bool networkHalt = true;

/****************************************************************************
 * Initialize_Network
 ***************************************************************************/
void Initialize_Network(void) {

    if(networkinitialized) return;

	s32 result;

    result = if_config(IP, NULL, NULL, true);

   if(result < 0) {
        networkinitialized = false;
		return;
	} else {
        networkinitialized = true;
        return;
	}
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
 * Get network IP
 ***************************************************************************/
bool ShutdownWC24()
{
    bool onlinefix = IsNetworkInit();
	if(onlinefix) {
		s32 kd_fd, ret;
		STACK_ALIGN(u8, kd_buf, 0x20, 32);

		kd_fd = IOS_Open("/dev/net/kd/request", 0);
		if (kd_fd >= 0) {
			ret = IOS_Ioctl(kd_fd, 7, NULL, 0, kd_buf, 0x20);
			if(ret >= 0)
				onlinefix = false; // fixed no IOS reload needed
			IOS_Close(kd_fd);
		}
	}
	return onlinefix;
}

s32 network_request(const char * request)
{
	char buf[1024];
	char *ptr = NULL;

	u32 cnt, size;
	s32 ret;

	/* Send request */
	ret = net_send(connection, request, strlen(request), 0);
	if (ret < 0)
		return ret;

	/* Clear buffer */
	memset(buf, 0, sizeof(buf));

	/* Read HTTP header */
	for (cnt = 0; !strstr(buf, "\r\n\r\n"); cnt++)
		if (net_recv(connection, buf + cnt, 1, 0) <= 0)
			return -1;

	/* HTTP request OK? */
	if (!strstr(buf, "HTTP/1.1 200 OK"))
		return -1;
	/* Retrieve content size */
	ptr = strstr(buf, "Content-Length:");
	if (!ptr)
		return -1;

	sscanf(ptr, "Content-Length: %u", &size);
	return size;
}

s32 network_read(u8 * buf, u32 len)
{
	u32 read = 0;
	s32 ret = -1;

	/* Data to be read */
	while (read < len) {
		/* Read network data */
		ret = net_read(connection, buf + read, len - read);
		if (ret < 0)
			return ret;

		/* Read finished */
		if (!ret)
			break;

		/* Increment read variable */
		read += ret;
	}

	return read;
}

/****************************************************************************
 * Download request
 ***************************************************************************/
s32 download_request(const char * url) {

    //Check if the url starts with "http://", if not it is not considered a valid url
	if(strncmp(url, "http://", strlen("http://")) != 0)
	{
		return -1;
	}

	//Locate the path part of the url by searching for '/' past "http://"
	char *path = strchr(url + strlen("http://"), '/');

	//At the very least the url has to end with '/', ending with just a domain is invalid
	if(path == NULL)
	{
		return -1;
	}

	//Extract the domain part out of the url
	int domainlength = path - url - strlen("http://");

	if(domainlength == 0)
	{
		return -1;
	}

	char domain[domainlength + 1];
	strncpy(domain, url + strlen("http://"), domainlength);
	domain[domainlength] = '\0';

	connection = GetConnection(domain);
    if(connection < 0) {
        return -1;
    }

    //Form a nice request header to send to the webserver
	char header[strlen(path)+strlen(domain)+100];
	sprintf(header, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, domain);

	s32 filesize = network_request(header);

    return filesize;
}

void CloseConnection() {

    net_close(connection);

}

/****************************************************************************
 * Update check
 ***************************************************************************/
int CheckUpdate()
{
    if(!networkinitialized)
        return -1;

    int revnumber = 0;
    int currentrev = atoi(SVN_REV);

    struct block file = downloadfile("http://www.techjawa.com/usbloadergx/rev.txt");
    char revtxt[10];

	u8  i;
    if(file.data != NULL) {
        for(i=0; i<9 || i < file.size; i++)
        revtxt[i] = file.data[i];
        revtxt[i] = 0;
        revnumber = atoi(revtxt);
        free(file.data);
	}

    if(revnumber > currentrev)
        return revnumber;
    else
        return -1;
}

/****************************************************************************
 * HaltNetwork
 ***************************************************************************/
void HaltNetworkThread()
{
    networkHalt = true;

	// wait for thread to finish
	while(!LWP_ThreadIsSuspended(networkthread))
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

/*********************************************************************************
 * Networkthread for background network initialize and update check with idle prio
 *********************************************************************************/
static void * networkinitcallback(void *arg)
{
    Initialize_Network();

    if(networkinitialized == true && updatechecked == false) {
        if(CheckUpdate() > 0) {
            /** Here we can enter the update function later **
             **  when network problem is solved             **/
            WindowPrompt("Update available",0,"OK",0,0,0);
        }
        updatechecked = true;
    }
	return NULL;
}

/****************************************************************************
 * InitNetworkThread with priority 0 (idle)
 ***************************************************************************/
void InitNetworkThread()
{
	LWP_CreateThread (&networkthread, networkinitcallback, NULL, NULL, 0, 0);
}

/****************************************************************************
 * ShutdownThread
 ***************************************************************************/
void ShutdownNetworkThread()
{
	LWP_JoinThread (networkthread, NULL);
	networkthread = LWP_THREAD_NULL;
}
