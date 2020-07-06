/*
    Code by blackb0x @ GBAtemp.net
    This allows the Wii to download from servers that use SNI.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <network.h>
#include <ogc/lwp_watchdog.h>
#include <unistd.h>

#include "https.h"
#include "gecko.h"
#include "../svnrev.h"
#include "picohttpparser.h"
#include "prompts/ProgressWindow.h"

u8 loop;
WOLFSSL_SESSION *session;

int https_write(HTTP_INFO *httpinfo, char *buffer, int len)
{
    int ret, slen = 0;
    while (1)
    {
        if (httpinfo->use_https)
            ret = wolfSSL_write(httpinfo->ssl, &buffer[slen], len - slen);
        else
            ret = net_write(httpinfo->sock, &buffer[slen], len - slen);

        if (ret == 0)
            continue;
        else if (ret <= 0)
            return ret; // Timeout would return -1

        slen += ret;
        if (slen >= len)
            break;
    }
    return slen;
}

int https_read(HTTP_INFO *httpinfo, char *buffer, int len)
{
    struct pollsd fds[1];
    fds[0].socket = httpinfo->sock;
    fds[0].events = POLLIN;

    net_fcntl(httpinfo->sock, F_SETFL, 4);
    switch (net_poll(fds, 1, READ_WRITE_TIMEOUT))
    {
    case -1:
#ifdef DEBUG_NETWORK
        gprintf("net_poll error\n");
#endif
        return -1;
    case 0:
#ifdef DEBUG_NETWORK
        gprintf("The connection timed out\n");
#endif
        return -ETIMEDOUT;
    default:
        net_fcntl(httpinfo->sock, F_SETFL, 0);
        if (len > 8192)
            len = 8192; // 16KB is the max on a Wii, but 8KB is safe
        if (httpinfo->use_https)
            return wolfSSL_read(httpinfo->ssl, buffer, len);
        return net_read(httpinfo->sock, buffer, len);
    }
}

void https_close(HTTP_INFO *httpinfo)
{
    if (httpinfo->use_https)
    {
        if (wolfSSL_shutdown(httpinfo->ssl) == SSL_SHUTDOWN_NOT_DONE)
            wolfSSL_shutdown(httpinfo->ssl);
        wolfSSL_free(httpinfo->ssl);
        wolfSSL_CTX_free(httpinfo->ctx);
    }
    net_close(httpinfo->sock);
#ifdef DEBUG_NETWORK
    gprintf("Closed socket and cleaned up\n");
#endif
}

u8 get_header_value(struct phr_header *headers, size_t num_headers, char *dst, char *header)
{
    for (size_t i = 0; i != num_headers; ++i)
    {
        if (strncasecmp(header, headers[i].name, headers[i].name_len) == 0)
        {
            strlcpy(dst, headers[i].value, headers[i].value_len + 1);
            return 1;
        }
    }
    return 0;
}

u64 get_header_value_int(struct phr_header *headers, size_t num_headers, char *header)
{
    char header_value[30] = {};
    if (!get_header_value(headers, num_headers, header_value, header))
        return 0;
    return strtoull(header_value, NULL, 0);
}

u8 is_chunked(struct phr_header *headers, size_t num_headers)
{
    char encoding[10] = {};
    if (!get_header_value(headers, num_headers, encoding, "transfer-encoding"))
        return 0;
    return (strcasecmp(encoding, "chunked") == 0) ? 1 : 0;
}

u8 read_chunked(HTTP_INFO *httpinfo, struct download *buffer, size_t start_pos)
{
    struct phr_chunked_decoder decoder = {};
    size_t capacity = 4096, rsize;
    ssize_t rret, pret;
    decoder.consume_trailer = 1;
#ifdef DEBUG_NETWORK
    gprintf("Data is chunked\n");
#endif
    do
    {
        if (buffer->show_progress)
        {
            if (ProgressCanceled())
                return 0;
            ShowProgress(start_pos, capacity); // Unknown size for chunked transfers
        }
        if (start_pos == capacity)
        {
#ifdef DEBUG_NETWORK
            gprintf("Increased buffer size\n");
#endif
            capacity *= 2;
            buffer->data = realloc(buffer->data, capacity);
        }
        while ((rret = https_read(httpinfo, &buffer->data[start_pos], capacity - start_pos)) == -1 && errno == EINTR)
            ;
        if (rret <= 0)
        {
#ifdef DEBUG_NETWORK
            gprintf("IO error\n");
#endif
            return 0;
        }
        rsize = rret;
        pret = phr_decode_chunked(&decoder, &buffer->data[start_pos], &rsize);
        if (pret == -1)
        {
#ifdef DEBUG_NETWORK
            gprintf("Parse error\n");
#endif
            return 0;
        }
        start_pos += rsize;
    } while (pret == -2);
    buffer->size = start_pos;
    buffer->data = realloc(buffer->data, buffer->size);
    return 1;
}

u8 read_all(HTTP_INFO *httpinfo, struct download *buffer, size_t start_pos)
{
    size_t capacity = 4096;
    ssize_t ret;
#ifdef DEBUG_NETWORK
    gprintf("Data is not chunked\n");
#endif
    while (1)
    {
        if (buffer->show_progress)
        {
            if (ProgressCanceled())
                return 0;
            ShowProgress(start_pos, buffer->content_length);
        }
        if (start_pos == capacity)
        {
#ifdef DEBUG_NETWORK
            gprintf("Increased buffer size\n");
#endif
            capacity *= 2;
            buffer->data = realloc(buffer->data, capacity);
        }
        while ((ret = https_read(httpinfo, &buffer->data[start_pos], capacity - start_pos)) == -1 && errno == EINTR)
            ;
        if (ret == 0)
            break;
        if (ret < 0)
            return 0;

        start_pos += ret;
    };
    buffer->size = start_pos;
    buffer->data = realloc(buffer->data, buffer->size);
    return 1;
}

int connect(char *host, u16 port)
{
    struct sockaddr_in sin;
    s32 sock, ret;
    u64 t;

    u32 ipaddress = getipbynamecached(host);
    if (ipaddress == 0)
        return -1;

    sock = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0)
        return sock;

    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = ipaddress;
#ifdef DEBUG_NETWORK
    gprintf("Connecting to %s (%s)\n", host, inet_ntoa(sin.sin_addr));
#endif
    net_fcntl(sock, F_SETFL, 4);
    t = gettime();
    while (1)
    {
        if (ticks_to_millisecs(diff_ticks(t, gettime())) > TCP_CONNECT_TIMEOUT)
        {
#ifdef DEBUG_NETWORK
            gprintf("The connection timed out\n");
#endif
            net_close(sock);
            return -ETIMEDOUT;
        }
        ret = net_connect(sock, (struct sockaddr *)&sin, sizeof(sin));
        if (ret < 0)
        {
            if (ret == -EISCONN)
                break;
            if (ret == -EINPROGRESS || ret == -EALREADY)
            {
                usleep(20 * 1000);
                continue;
            }
            net_close(sock);
            return ret;
        }
        break;
    }
    net_fcntl(sock, F_SETFL, 0);
    return sock;
}

void downloadfile(const char *url, struct download *buffer)
{
    HTTP_INFO httpinfo;
    memset(&httpinfo, 0, sizeof(HTTP_INFO));
    // Always reset the size due to the image downloader looping
    buffer->size = 0;

    // Check if we're using HTTPS and set the path
    char *path;
    if (strncmp(url, "https://", 8) == 0)
    {
        httpinfo.use_https = 1;
        path = strchr(url + 8, '/');
    }
    else if (strncmp(url, "http://", 7) == 0)
    {
        httpinfo.use_https = 0;
        path = strchr(url + 7, '/');
    }
    else
        return; // Prevents uninitialized warning

    if (path == NULL)
        return;

    // Get the host
    int domainlength = path - url - 7 - httpinfo.use_https;
    char host[domainlength + 1];
    strlcpy(host, url + 7 + httpinfo.use_https, domainlength + 1);

    // Start connecting
    if ((httpinfo.sock = connect(host, httpinfo.use_https ? 443 : 80)) < 0)
    {
#ifdef DEBUG_NETWORK
        gprintf("Failed to connect to %s\n", host);
#endif
        return;
    }
#ifdef DEBUG_NETWORK
    else
        gprintf("Connected\n");
#endif

    if (httpinfo.use_https)
    {
        // Create a new SSL context
        // wolfSSLv23_client_method() works, but resume would require further changes
        if ((httpinfo.ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method())) == NULL)
        {
#ifdef DEBUG_NETWORK
            gprintf("Failed to create WOLFSSL_CTX\n");
#endif
            https_close(&httpinfo);
            return;
        }
        // Don't verify certificates
        wolfSSL_CTX_set_verify(httpinfo.ctx, WOLFSSL_VERIFY_NONE, 0);
        // Enable SNI
        if (wolfSSL_CTX_UseSNI(httpinfo.ctx, 0, host, strlen(host)) != WOLFSSL_SUCCESS)
        {
#ifdef DEBUG_NETWORK
            gprintf("Failed to set SNI\n");
#endif
            https_close(&httpinfo);
            return;
        }
        // Create a new wolfSSL session
        if ((httpinfo.ssl = wolfSSL_new(httpinfo.ctx)) == NULL)
        {
#ifdef DEBUG_NETWORK
            gprintf("SSL session creation failed\n");
#endif
            https_close(&httpinfo);
            return;
        }
        // Set the file descriptor
        if (wolfSSL_set_fd(httpinfo.ssl, httpinfo.sock) != SSL_SUCCESS)
        {
#ifdef DEBUG_NETWORK
            gprintf("Failed to set SSL file descriptor\n");
#endif
            https_close(&httpinfo);
            return;
        }
        // Attempt to resume the session
        if (session != NULL && wolfSSL_set_session(httpinfo.ssl, session) != SSL_SUCCESS)
        {
#ifdef DEBUG_NETWORK
            gprintf("Failed to set session (session timed out?)\n");
#endif
            session = NULL;
        }
        // Initiate a handshake
        if (wolfSSL_connect(httpinfo.ssl) != SSL_SUCCESS)
        {
#ifdef DEBUG_NETWORK
            gprintf("SSL handshake failed\n");
#endif
            https_close(&httpinfo);
            return;
        }
        // Check if we resumed successfully
        if (session != NULL && !wolfSSL_session_reused(httpinfo.ssl))
        {
#ifdef DEBUG_NETWORK
            gprintf("Failed to resume session\n");
#endif
            session = NULL;
        }
        // Cipher info
#ifdef DEBUG_NETWORK
        /*char ciphers[4096];
        wolfSSL_get_ciphers(ciphers, (int)sizeof(ciphers));
        gprintf("All supported ciphers: %s\n", ciphers);*/
        WOLFSSL_CIPHER *cipher = wolfSSL_get_current_cipher(httpinfo.ssl);
        gprintf("Using: %s - %s\n", wolfSSL_get_version(httpinfo.ssl), wolfSSL_CIPHER_get_name(cipher));
#endif
    }

    // Send our request
    char request[2300];
    char isgecko[36] = "Cookie: challenge=BitMitigate.com\r\n";
    char method[5] = "HEAD"; // The only way to get the GameTDB timestamp
    int ret, len;
    if (strcmp(host, "www.geckocodes.org") != 0)
        memset(isgecko, 0, sizeof(isgecko)); // Not geckocodes, so don't set a cookie

    if (!buffer->gametdbcheck)
        strcpy(method, "GET");

    len = snprintf(request, 2300,
                   "%s %s HTTP/1.1\r\n"
                   "Host: %s\r\n"
                   "User-Agent: USBLoaderGX/%s\r\n"
                   "Connection: close\r\n"
                   "%s"
                   "Pragma: no-cache\r\n"
                   "Cache-Control: no-cache\r\n\r\n",
                   method, path, host, GetRev(), isgecko);
    if ((ret = https_write(&httpinfo, request, len)) != len)
    {
#ifdef DEBUG_NETWORK
        gprintf("https_write error: %i\n", ret);
#endif
        https_close(&httpinfo);
        return;
    }

    // Check if we want a response
    if (buffer->skip_response)
    {
#ifdef DEBUG_NETWORK
        gprintf("Sent request to %s and skipping response\n", host);
#endif
        https_close(&httpinfo);
        return;
    }

    // Get the response
    char response[4096];
    struct phr_header headers[100];
    int pret, minor_version, status, dl_valid;
    size_t buflen = 0, prevbuflen = 0, num_headers, msg_len;
    ssize_t rret;
    const char *msg;

    while (1)
    {
        // Read the response
        while ((rret = https_read(&httpinfo, &response[buflen], 1)) == -1 && errno == EINTR)
            ;
        if (rret <= 0)
        {
#ifdef DEBUG_NETWORK
            gprintf("rret error %i\n", rret);
#endif
            https_close(&httpinfo);
            return;
        }
        prevbuflen = buflen;
        buflen += rret;
        // Parse the response
        num_headers = sizeof(headers) / sizeof(headers[0]);
        pret = phr_parse_response(response, buflen, &minor_version, &status, &msg, &msg_len, headers, &num_headers, prevbuflen);
        if (pret > 0)
            break; // Successfully parsed the response
        else if (pret == -1)
        {
#ifdef DEBUG_NETWORK
            gprintf("pret error %i\n", pret);
#endif
            https_close(&httpinfo);
            return;
        }
        // Response is incomplete so continue the loop
        if (buflen == sizeof(response))
        {
#ifdef DEBUG_NETWORK
            gprintf("buflen error %i\n", buflen);
#endif
            https_close(&httpinfo);
            return;
        }
    }

    // The website wants to redirect us
    if (status == 301 || status == 302)
    {
        https_close(&httpinfo);
        if (loop == REDIRECT_LIMIT)
        {
#ifdef DEBUG_NETWORK
            gprintf("Reached redirect limit\n");
#endif
            return;
        }
        loop++;
        char location[2100] = {};
        if (!get_header_value(headers, num_headers, location, "location"))
            return;
#ifdef DEBUG_NETWORK
        gprintf("Redirect #%i - %s\n", loop, location);
#endif
        downloadfile(location, buffer);
        return;
    }
    // It's not 301 or 302, so reset the loop
    loop = 0;
    // Exit if it's a GameTDB HEAD request
    if (buffer->gametdbcheck)
    {
        buffer->gametdbcheck = get_header_value_int(headers, num_headers, "x-gametdb-timestamp");
        https_close(&httpinfo);
        return;
    }
    // We got what we wanted
    if (status == 200)
    {
        buffer->data = malloc(4096);
        memcpy(buffer->data, &response[pret], buflen - pret);
        if (buffer->show_progress)
            buffer->content_length = get_header_value_int(headers, num_headers, "content-length");
        // Determine how to read the data
        if (is_chunked(headers, num_headers))
            dl_valid = read_chunked(&httpinfo, buffer, buflen - pret);
        else
            dl_valid = read_all(&httpinfo, buffer, buflen - pret);
        // Check if the download is incomplete
        if (!dl_valid || buffer->size <= 0)
        {
            buffer->size = 0;
            free(buffer->data);
#ifdef DEBUG_NETWORK
            gprintf("Removed incomplete download\n");
#endif
            https_close(&httpinfo);
            return;
        }
        // Save the session
        if (httpinfo.use_https)
            session = wolfSSL_get_session(httpinfo.ssl);
        // Finished
        https_close(&httpinfo);
#ifdef DEBUG_NETWORK
        gprintf("Download size: %llu\n", buffer->size);
        gprintf("Headers:\n");
        for (size_t i = 0; i != num_headers; ++i)
            gprintf("%.*s: %.*s\n", (int)headers[i].name_len, headers[i].name, (int)headers[i].value_len, headers[i].value);
#endif
        return;
    }
    // Close on all other status codes
#ifdef DEBUG_NETWORK
    gprintf("Status code: %i - %s\n", status, url);
#endif
    https_close(&httpinfo);
}
