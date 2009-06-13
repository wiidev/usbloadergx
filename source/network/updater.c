#include <stdio.h>
#include <string.h>
#include <ogcsys.h>

#include "http.h"

static s32 connection;
bool netcheck = false;

/*Networking - Forsaekn*/
int Net_Init(char *ip){

	s32 res;
    while ((res = net_init()) == -EAGAIN)
	{
		usleep(100 * 1000); //100ms
	}

    if (if_config(ip, NULL, NULL, true) < 0) {
		printf("      Error reading IP address, exiting");
		usleep(1000 * 1000 * 1); //1 sec
		return FALSE;
	}
	return TRUE;
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

s32 network_read(void *buf, u32 len)
{
	s32 read = 0, ret;

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

s32 downloadrev(const char * url) {

    //Check if the url starts with "http://", if not it is not considered a valid url
	if(strncmp(url, "http://", strlen("http://")) != 0)
	{
		//printf("URL '%s' doesn't start with 'http://'\n", url);
		return -1;
	}

	//Locate the path part of the url by searching for '/' past "http://"
	char *path = strchr(url + strlen("http://"), '/');

	//At the very least the url has to end with '/', ending with just a domain is invalid
	if(path == NULL)
	{
		//printf("URL '%s' has no PATH part\n", url);
		return -1;
	}

	//Extract the domain part out of the url
	int domainlength = path - url - strlen("http://");

	if(domainlength == 0)
	{
		//printf("No domain part in URL '%s'\n", url);
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
	char* headerformat = "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n";;
	char header[strlen(headerformat) + strlen(domain) + strlen(path)];
	sprintf(header, headerformat, path, domain);

	s32 filesize = network_request(header);

    return filesize;
}

void CloseConnection() {

    net_close(connection);

}
