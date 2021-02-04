/****************************************************************************
 * Network Operations
 * for USB Loader GX
 *
 * HTTP operations
 * Written by dhewg/bushing modified by dimok
 ****************************************************************************/

#include <ogcsys.h>

#include "networkops.h"
#include "https.h"
#include "update.h"
#include "gecko.h"
#include "settings/ProxySettings.h"

#define PORT 4299

/*** Incomming filesize ***/
u32 infilesize = 0;
u32 uncfilesize = 0;

s32 connection;
static s32 socket;
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

	if (networkinitialized)
		return;

	s32 result;

	result = if_config(IP, NULL, NULL, true, retries);

	if (result < 0)
	{
		networkinitialized = false;
		return;
	}
	else
	{
		getProxyInfo();
		wolfSSL_Init();
		networkinitialized = true;
		gprintf("Initialized network\n");
		return;
	}
}

/****************************************************************************
 * DeinitNetwork
 ***************************************************************************/
void DeinitNetwork(void)
{
	wolfSSL_Cleanup();
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
char *GetNetworkIP(void)
{
	return IP;
}

/****************************************************************************
 * Get incomming IP
 ***************************************************************************/
char *GetIncommingIP(void)
{
	return incommingIP;
}

/****************************************************************************
 * Read network data
 ***************************************************************************/
s32 network_read(s32 connect, u8 *buf, u32 len)
{
	if (connect == NET_DEFAULT_SOCK)
		connect = connection;

	u32 read = 0;
	s32 ret = -1;

	/* Data to be read */
	while (read < len)
	{
		/* Read network data */
		ret = net_read(connect, buf + read, len - read);
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
 * Close the connection
 ***************************************************************************/
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
 * NetworkWait
 ***************************************************************************/
int NetworkWait()
{
	if (!checkincomming)
		return -3;

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
	sin.sin_port = htons(PORT);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	if (net_bind(socket, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		net_close(socket);
		return -1;
	}

	if (net_listen(socket, 3) < 0)
	{
		net_close(socket);
		return -1;
	}

	connection = net_accept(socket, (struct sockaddr *)&client_address, &addrlen);

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

	if (waitforanswer)
		CloseConnection();

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
 * Networkthread for background network initialize
 *********************************************************************************/
static void *networkinitcallback(void *arg)
{
	while (1)
	{
		if (!checkincomming && networkHalt)
			LWP_SuspendThread(networkthread);

		Initialize_Network();

		if (networkinitialized)
			networkHalt = true;

		if (checkincomming)
			NetworkWait();

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
