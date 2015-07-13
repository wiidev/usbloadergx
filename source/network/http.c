/*  http -- http convenience functions

    Copyright (C) 2008 bushing
				  2008-2014 Dimok
				  2015 Fix94 ; Cyan

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 2.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <ogc/lwp_watchdog.h>

#include "http.h"
#include "ssl.h"
#include "../svnrev.h"
#include "prompts/ProgressWindow.h"
#include "language/gettext.h"
#include "gecko.h"

extern char incommingIP[50];
static u8 retryloop = 0;
static bool displayProgressWindow = false;

http_res result;
u16 http_port;
u32 http_status;
u32 content_length;
char content_location[255] = "";

/**
 * Emptyblock is a statically defined variable for functions to return if they are unable
 * to complete a request
 */
const struct block emptyblock = { 0, NULL };

// Write our message to the server
static s32 tcp_write(s32 server, char *msg)
{
	s32 bytes_transferred = 0;
	s32 remaining = strlen(msg);
	s32 res = 0;
	while (remaining)
	{
		if(http_port == 443)
			res = (bytes_transferred = ssl_write(server, msg, remaining > NET_BUFFER_SIZE ? NET_BUFFER_SIZE : remaining));
		else
			res = (bytes_transferred = net_write(server, msg, remaining > NET_BUFFER_SIZE ? NET_BUFFER_SIZE : remaining));
		if (res > 0)
		{
			remaining -= bytes_transferred;
			msg += bytes_transferred;
			usleep(20 * 1000);
		}
		else if (bytes_transferred < 0)
		{
			return bytes_transferred;
		}
		else
		{
			return -ENODATA;
		}
	}
	return 0;
}

/**
 * Connect to a remote server via TCP on a specified port
 *
 * @param u32 ip address of the server to connect to
 * @param u32 the port to connect to on the server
 * @return s32 The connection to the server (negative number if connection could not be established)
 */
static s32 tcp_connect(u32 ipaddress, u16 socket_port)
{
	//Initialize socket
	s32 connection = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (connection < 0) return connection;

	struct sockaddr_in connect_addr;
	memset(&connect_addr, 0, sizeof(connect_addr));
	connect_addr.sin_family = AF_INET;
	connect_addr.sin_port = htons(socket_port);
	connect_addr.sin_addr.s_addr = ipaddress;

	sprintf(incommingIP, "%s", inet_ntoa(connect_addr.sin_addr));

	//Attempt to open the socket
	if (net_connect(connection, (struct sockaddr*) &connect_addr, sizeof(connect_addr)) == -1)
	{
		net_close(connection);
		return -1;
	}
	return connection;
}

/**
 * Reads the current connection data line by line
 *
 * @return string up to "\r\n" or until max_length is reached.
 */
char * tcp_readLine (const s32 s, const u16 max_length, const u64 start_time, const u16 timeout) {
	char *buf;
	u16 c;
	s32 res;
	char *ret;

	buf = malloc(max_length);

	c = 0;
	ret = NULL;
	while (true) {
		if (ticks_to_millisecs (diff_ticks (start_time, gettime ())) > timeout)
			break;

		if(http_port == 443)
			res = ssl_read (s, &buf[c], 1);
		else
			res = net_read (s, &buf[c], 1);

		if ((res == 0) || (res == -EAGAIN)) {
			usleep (20 * 1000);

			continue;
		}

		if (res < 0) break;

		if ((c > 0) && (buf[c - 1] == '\r') && (buf[c] == '\n')) {
			if (c == 1) {
				ret = strdup ("");

				break;
			}

			ret = strndup (buf, c - 1);

			break;
		}

		c++;

		if (c == max_length)
			break;
	}

	free (buf);
	return ret;
}

/**
 * This function reads all the data from a connection into a buffer.
 *
 * @param s32 connection The connection identifier to suck the response out of
 * @return bool True if data downloaded succesfully.
 */
bool tcp_readData(const s32 connection, u8 **buffer, const u32 length)
{
	u8 *p;
	u32 left, block, received;
	s64 t;
	s32 res;

	p = *buffer;
	left = length;
	received = 0;

	t = gettime ();
	while (left)
	{
		if (ticks_to_millisecs (diff_ticks (t, gettime ())) > TCP_BLOCK_RECV_TIMEOUT)
			break;

		// Update the progress bar
		if(displayProgressWindow)
		{
			ShowProgress(received, length);
			if(ProgressCanceled())
			{
				ProgressStop();
				break;
			}
		}

		// Get next block size
		block = left;
		if (block > TCP_BLOCK_SIZE)
			block = TCP_BLOCK_SIZE;

		if(http_port == 443)
			res = ssl_read (connection, p, block);
		else
			res = net_read (connection, p, block);

		if ((res == 0) || (res == -EAGAIN))
		{
			usleep (20 * 1000);
			continue;
		}

		if (res < 0) break;

		received += res;
		left -= res;
		p += res;

		// update timing after each downloaded block
		t = gettime ();
	}

	return left == 0;
}

/**
 * Parse the HTTP replied header
 *
 * @param s32 connection identification
 * @return bool true if max header lines not reached. Stores HTTP answers into global variables.
 */
u8 read_header(s32 connection)
{
	int linecount = 0;
	result = HTTPR_OK;

	for (linecount = 0; linecount < 32; linecount++)
	{
		char *line = tcp_readLine (connection, 0xff, gettime(), (u16)HTTP_TIMEOUT);

		if (!line)
		{
			http_status = 404;
			result = HTTPR_ERR_REQUEST;
			break;
		}

		if (strlen (line) < 1)
		{
			free (line);
			line = NULL;
			break;
		}

		sscanf (line, "HTTP/1.%*u %u", &http_status);
		sscanf (line, "Content-Length: %u", &content_length);
		sscanf (line, "Location: %s", content_location);
		//gprintf(line);
		//gprintf("\n");
		free (line);
		line = NULL;
	}

	if (linecount == 32)
	{
		http_status = 404;
		result = HTTPR_ERR_REQUEST;
	}

	return result;
}

/**
 * Downloads the contents of a URL to memory
 * This method is not threadsafe (because networking is not threadsafe on the Wii)
 */
struct block downloadfile(const char *url)
{
	int sslcontext = -1;

	//Check if the url starts with "http://", if not it is not considered a valid url
	if (strncmp(url, "http://", strlen("http://")) == 0)
		http_port = 80;
	else if(strncmp(url, "https://", strlen("https://")) == 0)
	{
		http_port = 443;
		gprintf("Initializing ssl...\n");
		if(ssl_init() < 0)
			return emptyblock;
	}
	else
		return emptyblock;

	//Locate the path part of the url by searching for '/' past "http://"
	char *path = 0;
	if(http_port == 443)
		path = strchr(url + strlen("https://"), '/');
	else
		path = strchr(url + strlen("http://"), '/');

	//At the very least the url has to end with '/', ending with just a domain is invalid
	if (path == NULL)
	{
		//printf("URL '%s' has no PATH part\n", url);
		return emptyblock;
	}

	//Extract the domain part out of the url
	int domainlength = path - url - strlen("http://") - (http_port == 443 ? 1 : 0);

	if (domainlength == 0)
	{
		//printf("No domain part in URL '%s'\n", url);
		return emptyblock;
	}

	char domain[domainlength + 1];
	strlcpy(domain, url + strlen("http://") + (http_port == 443 ? 1 : 0), domainlength + 1);

	//Parsing of the URL is done, start making an actual connection
	u32 ipaddress = getipbynamecached(domain);

	if (ipaddress == 0)
	{
		//printf("\ndomain %s could not be resolved", domain);
		return emptyblock;
	}

	s32 connection = tcp_connect(ipaddress, http_port);

	if (connection < 0)
	{
		//printf("Error establishing connection");
		return emptyblock;
	}

	if(http_port == 443)
	{
		//patched out anyways so just to set something
		sslcontext = ssl_new((u8*)domain,0);

		if(sslcontext < 0)
		{
			//gprintf("ssl_new\n");
			result = HTTPR_ERR_CONNECT;
			net_close (connection);
			return emptyblock;
		}
		//patched out anyways so just to set something
		ssl_setbuiltinclientcert(sslcontext,0);
		if(ssl_connect(sslcontext,connection) < 0)
		{
			//gprintf("ssl_connect\n");
			result = HTTPR_ERR_CONNECT;
			ssl_shutdown(sslcontext);
			net_close (connection);
			return emptyblock;
		}
		int ret = ssl_handshake(sslcontext);
		if(ret < 0)
		{
			//gprintf("ssl_handshake %i\n", ret);
			result = HTTPR_ERR_STATUS;
			ssl_shutdown(sslcontext);
			net_close (connection);
			return emptyblock;
		}
	}

	// Remove Referer from the request header for incompatible websites (ex. Cloudflare proxy)
	char referer[domainlength + 12];
	snprintf(referer, sizeof(referer), "Referer: %s\r\n", domain);
	if(strstr(url, "geckocodes"))
	{
		strcpy(referer, "");
	}

	//Form a nice request header to send to the webserver
	char* headerformat = "GET %s HTTP/1.0\r\nHost: %s\r\n%sUser-Agent: USBLoaderGX r%s\r\n\r\n";
	char header[strlen(headerformat) + strlen(path) + strlen(domain) + strlen(referer) + 100];
	sprintf(header, headerformat, path, domain, referer, GetRev());
	//gprintf("\nHTTP Request:\n");
	//gprintf("%s\n",header);

	//Do the request and get the response
	tcp_write(http_port == 443 ? sslcontext : connection, header);
	read_header( http_port == 443 ? sslcontext : connection);

	if (http_status >= 400) // Not found
	{
		//gprintf("HTTP ERROR: %d\n", http_status);
		return emptyblock;
	}

	if(!content_length)
		content_length = 0;

	// create data buffer to return
	struct block response;
	response.data = malloc(content_length);
	response.size = content_length;
	if (response.data == NULL)
	{
		return emptyblock;
	}

	if (http_status == 200)
	{
		if(displayProgressWindow)
		{
			ProgressCancelEnable(true);
			StartProgress(tr("Downloading file..."), tr("Please wait"), 0, false, false);
		}

		int ret = tcp_readData(http_port == 443 ? sslcontext : connection, &response.data, content_length);
		if(!ret)
		{
			free(response.data);
			result = HTTPR_ERR_RECEIVE;
			if(http_port == 443)
				ssl_shutdown(sslcontext);
			net_close (connection);
			return emptyblock;
		}
	}
	else if (http_status == 302) // 302 FOUND (redirected link)
	{
		// close current connection
		if(http_port == 443)
			ssl_shutdown(sslcontext);
		net_close (connection);

		// prevent infinite loops
		retryloop++;
		if(retryloop > 3)
		{
			retryloop = 0;
			return emptyblock;
		}

		struct block redirected = downloadfile(content_location);
		if(redirected.size == 0)
			return emptyblock;

		// copy the newURL data into the original data
		u8 * tmp = realloc(response.data, redirected.size);
		if (tmp == NULL)
		{
			gprintf("Could not allocate enough memory for new URL. Download canceled.\n");
			free(response.data);
			response.size = 0;
			free(redirected.data);
			result = HTTPR_ERR_RECEIVE;
			if(http_port == 443)
				ssl_shutdown(sslcontext);
			net_close (connection);
			return emptyblock;
		}
		response.data = tmp;
		memcpy(response.data, redirected.data, redirected.size);
		free(redirected.data);
		response.size = redirected.size;

	}
	retryloop = 0;
	
	// reset progress window if used
	if(displayProgressWindow)
	{
		ProgressStop();
		ProgressCancelEnable(false);
		displayProgressWindow = false;
	}

	return response;
}

s32 GetConnection(char * domain)
{

	u32 ipaddress = getipbynamecached(domain);
	if (ipaddress == 0)
	{
		return -1;
	}
	s32 connection = tcp_connect(ipaddress, 80);
	return connection;

}

void displayDownloadProgress(bool display)
{
	displayProgressWindow = display;
}