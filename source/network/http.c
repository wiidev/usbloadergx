#include "http.h"
#include "../svnrev.h"
#include "gecko.h"

extern char incommingIP[50];
static u8 retryloop = 0;

/**
 * Emptyblock is a statically defined variable for functions to return if they are unable
 * to complete a request
 */
const struct block emptyblock = { 0, NULL };

//The maximum amount of bytes to send per net_write() call
//#define NET_BUFFER_SIZE 1024
#define NET_BUFFER_SIZE 3600

// Write our message to the server
static s32 send_message(s32 server, char *msg)
{
	s32 bytes_transferred = 0;
	s32 remaining = strlen(msg);
	while (remaining)
	{
		if ((bytes_transferred = net_write(server, msg, remaining > NET_BUFFER_SIZE ? NET_BUFFER_SIZE : remaining)) > 0)
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
static s32 server_connect(u32 ipaddress, u16 socket_port)
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

	//Attemt to open the socket
	if (net_connect(connection, (struct sockaddr*) &connect_addr, sizeof(connect_addr)) == -1)
	{
		net_close(connection);
		return -1;
	}
	return connection;
}

//The amount of memory in bytes reserved initially to store the HTTP response in
//Be careful in increasing this number, reading from a socket on the Wii
//will fail if you request more than 20k or so
#define HTTP_BUFFER_SIZE 1024 * 5

//The amount of memory the buffer should expanded with if the buffer is full
#define HTTP_BUFFER_GROWTH 1024 * 5

/**
 * This function reads all the data from a connection into a buffer which it returns.
 * It will return an empty buffer if something doesn't go as planned
 *
 * @param s32 connection The connection identifier to suck the response out of
 * @return block A 'block' struct (see http.h) in which the buffer is located
 */
struct block read_message(s32 connection)
{
	//Create a block of memory to put in the response
	struct block buffer;
	buffer.data = malloc(HTTP_BUFFER_SIZE);
	buffer.size = HTTP_BUFFER_SIZE;

	if (buffer.data == NULL)
	{
		return emptyblock;
	}

	//The offset variable always points to the first byte of memory that is free in the buffer
	u32 offset = 0;

	while (1)
	{
		//Fill the buffer with a new batch of bytes from the connection,
		//starting from where we left of in the buffer till the end of the buffer
		s32 bytes_read = net_read(connection, buffer.data + offset, buffer.size - offset);

		//Anything below 0 is an error in the connection
		if (bytes_read < 0)
		{
			//printf("Connection error from net_read()  Errorcode: %i\n", bytes_read);
			return emptyblock;
		}

		//No more bytes were read into the buffer,
		//we assume this means the HTTP response is done
		if (bytes_read == 0)
		{
			break;
		}

		offset += bytes_read;

		//Check if we have enough buffer left over,
		//if not expand it with an additional HTTP_BUFFER_GROWTH worth of bytes
		if (offset >= buffer.size)
		{
			buffer.size += HTTP_BUFFER_GROWTH;
			u8 * tmp = realloc(buffer.data, buffer.size);

			if (tmp == NULL)
			{
				free(buffer.data);
				return emptyblock;
			}
			else
				buffer.data = tmp;
		}
	}

	//At the end of above loop offset should be precisely the amount of bytes that were read from the connection
	buffer.size = offset;

	//Shrink the size of the buffer so the data fits exactly in it
	buffer.data = realloc(buffer.data, buffer.size);

	return buffer;
}

/**
 * Downloads the contents of a URL to memory
 * This method is not threadsafe (because networking is not threadsafe on the Wii)
 */
struct block downloadfile(const char *url)
{
	//Check if the url starts with "http://", if not it is not considered a valid url
	if (strncmp(url, "http://", strlen("http://")) != 0)
	{
		//printf("URL '%s' doesn't start with 'http://'\n", url);
		return emptyblock;
	}

	//Locate the path part of the url by searching for '/' past "http://"
	char *path = strchr(url + strlen("http://"), '/');

	//At the very least the url has to end with '/', ending with just a domain is invalid
	if (path == NULL)
	{
		//printf("URL '%s' has no PATH part\n", url);
		return emptyblock;
	}

	//Extract the domain part out of the url
	int domainlength = path - url - strlen("http://");

	if (domainlength == 0)
	{
		//printf("No domain part in URL '%s'\n", url);
		return emptyblock;
	}

	char domain[domainlength + 1];
	strlcpy(domain, url + strlen("http://"), domainlength + 1);

	//Parsing of the URL is done, start making an actual connection
	u32 ipaddress = getipbynamecached(domain);

	if (ipaddress == 0)
	{
		//printf("\ndomain %s could not be resolved", domain);
		return emptyblock;
	}

	s32 connection = server_connect(ipaddress, 80);

	if (connection < 0)
	{
		//printf("Error establishing connection");
		return emptyblock;
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
	send_message(connection, header);
	struct block response = read_message(connection);
	net_close(connection);
	
	// dump response
	// hexdump(response.data, response.size);
	
	//Search for the 4-character sequence \r\n\r\n in the response which signals the start of the http payload (file)
	unsigned char *filestart = NULL;
	u32 filesize = 0;
	u32 i;
	char newURL[512];
	bool redirect = false;

	for (i = 3; i < response.size; i++)
	{
		if (response.data[i] == '\n' && response.data[i - 1] == '\r' && response.data[i - 2] == '\n' && response.data[i - 3] == '\r')
		{
			filestart = response.data + i + 1;
			filesize = response.size - i - 1;
			
			// Check the HTTP response code
			if (response.size > 10 && strncmp((char*)response.data, "HTTP/", 5)==0) 
			{
				char htstat[i];
				strncpy(htstat, (char*)response.data, i);
				htstat[i] = 0;
				char *codep;
				codep = strchr(htstat, ' ');
				if (codep) 
				{
					int code;
					if (sscanf(codep+1, "%d", &code) == 1) 
					{
						//gprintf("HTTP response code: %d\n", code);
						if (code == 302) // 302 FOUND (redirected link)
						{
							char *ptr = strcasestr((char*)response.data, "Location: ");
							if(ptr)
							{
								ptr += strlen("Location: ");
								strncpy(newURL, ptr, sizeof(newURL));
								*(strchr(newURL, '\r'))=0;
								
								redirect = true;
								//gprintf("New URL to download = %s \n", newURL);
							}
							else
							{
								//gprintf("HTTP ERROR: %s\n", htstat);
								free(response.data);
								return emptyblock;
							}
						}
						if (code >=400) // Not found
						{
							//gprintf("HTTP ERROR: %s\n", htstat);
							free(response.data);
							return emptyblock;
						}
					}
				}
			}
			break;
		}
	}

	if(redirect)
	{
		// Prevent endless loop
		retryloop++;
		if(retryloop > 3)
		{
			retryloop = 0;
			free(response.data);
			return emptyblock;
		}
		
		struct block redirected = downloadfile(newURL);

		// copy the newURL data into the original data
		u8 * tmp = realloc(response.data, redirected.size);
		if (tmp == NULL)
		{
			//gprintf("Could not allocate enough memory for new URL. Download canceled.\n");
			free(response.data);
			free(redirected.data);
			return emptyblock;
		}
		response.data = tmp;
		memcpy(response.data, redirected.data, redirected.size);

		// Set filestart's new size based on redirected file
		filestart = response.data;
		filesize = redirected.size;
		free(redirected.data);
		
	}
	retryloop = 0;

	if (filestart == NULL)
	{
		//printf("HTTP Response was without a file\n");
		free(response.data);
		return emptyblock;
	}

	//Copy the file part of the response into a new memoryblock to return
	struct block file;
	file.data = malloc(filesize);
	file.size = filesize;

	if (file.data == NULL)
	{
		//printf("No more memory to copy file from HTTP response\n");
		free(response.data);
		return emptyblock;
	}

	memcpy(file.data, filestart, filesize);

	//Dispose of the original response
	free(response.data);

	return file;
}

s32 GetConnection(char * domain)
{

	u32 ipaddress = getipbynamecached(domain);
	if (ipaddress == 0)
	{
		return -1;
	}
	s32 connection = server_connect(ipaddress, 80);
	return connection;

}
