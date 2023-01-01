/*
    Code by blackb0x @ GBAtemp.net
    This allows the Wii to download from servers that use SNI.
*/
#include <network.h>
#include <ogc/lwp_watchdog.h>

#include "../svnrev.h"
#include "gecko.h"
#include "https.h"
#include "prompts/ProgressWindow.h"
#include "settings/ProxySettings.h"
#include "utils/base64.h"

u8 loop;
WOLFSSL_SESSION *session;

int https_write(HTTP_INFO *httpinfo, char *buffer, int len, bool proxy)
{
    int ret, pos = 0;
    int rlen = len > BLOCK_SIZE ? BLOCK_SIZE : len;
    u64 time = gettime();
    while (ticks_to_millisecs(diff_ticks(time, gettime())) < READ_WRITE_TIMEOUT)
    {
        if (httpinfo->use_https && !proxy)
            ret = wolfSSL_write(httpinfo->ssl, &buffer[pos], rlen);
        else
            ret = net_write(httpinfo->sock, &buffer[pos], rlen);
        if (ret > 0)
        {
            pos += ret;
            rlen = len - pos > BLOCK_SIZE ? BLOCK_SIZE : len - pos;
            if (pos >= len)
                return pos;
            time = gettime();
        }
        usleep(10000);
    }
#ifdef DEBUG_NETWORK
    gprintf("The connection timed out (write)\n");
#endif
    return -ETIMEDOUT;
}

int https_read(HTTP_INFO *httpinfo, char *buffer, int len, bool proxy)
{
    int ret = -ETIMEDOUT;
    u64 time = gettime();
    if (len > BLOCK_SIZE)
        len = BLOCK_SIZE;
    while (ticks_to_millisecs(diff_ticks(time, gettime())) < READ_WRITE_TIMEOUT)
    {
        if (httpinfo->use_https && !proxy)
            ret = wolfSSL_read(httpinfo->ssl, buffer, len);
        else
            ret = net_read(httpinfo->sock, buffer, len);
        if (ret >= 0)
            return ret;
        usleep(10000);
    }
#ifdef DEBUG_NETWORK
    gprintf("The connection timed out (read)\n");
#endif
    return -ETIMEDOUT;
}

int send_callback(__attribute__((unused)) WOLFSSL *ssl, char *buf, int sz, void *ctx)
{
    int sent = net_write(*(int *)ctx, buf, sz);
    if (sent < 0)
    {
        if (sent == -EAGAIN)
            return WOLFSSL_CBIO_ERR_WANT_WRITE;
        else if (sent == -ECONNRESET)
            return WOLFSSL_CBIO_ERR_CONN_RST;
        else if (sent == -EINTR)
            return WOLFSSL_CBIO_ERR_ISR;
        else if (sent == -EPIPE)
            return WOLFSSL_CBIO_ERR_CONN_CLOSE;
        else
            return WOLFSSL_CBIO_ERR_GENERAL;
    }
    return sent;
}

int recv_callback(__attribute__((unused)) WOLFSSL *ssl, char *buf, int sz, void *ctx)
{
    int recvd = net_read(*(int *)ctx, buf, sz);
    if (recvd < 0)
    {
        if (recvd == -EAGAIN)
            return WOLFSSL_CBIO_ERR_WANT_READ;
        else if (recvd == -ECONNRESET)
            return WOLFSSL_CBIO_ERR_CONN_RST;
        else if (recvd == -EINTR)
            return WOLFSSL_CBIO_ERR_ISR;
        else if (recvd == -ECONNABORTED)
            return WOLFSSL_CBIO_ERR_CONN_CLOSE;
        else
            return WOLFSSL_CBIO_ERR_GENERAL;
    }
    else if (recvd == 0)
        return WOLFSSL_CBIO_ERR_CONN_CLOSE;
    return recvd;
}

void https_close(HTTP_INFO *httpinfo)
{
    if (httpinfo->use_https)
    {
        wolfSSL_shutdown(httpinfo->ssl);
        wolfSSL_free(httpinfo->ssl);
        wolfSSL_CTX_free(httpinfo->ctx);
    }
    net_close(httpinfo->sock);
#ifdef DEBUG_NETWORK
    gprintf("Closed socket and cleaned up\n");
#endif
}

bool get_header_value(struct phr_header *headers, size_t num_headers, char *dst, char *header)
{
    for (size_t i = 0; i != num_headers; ++i)
    {
        if (strncasecmp(header, headers[i].name, headers[i].name_len) == 0)
        {
            strlcpy(dst, headers[i].value, headers[i].value_len + 1);
            return true;
        }
    }
    return false;
}

u64 get_header_value_int(struct phr_header *headers, size_t num_headers, char *header)
{
    char header_value[30];
    if (!get_header_value(headers, num_headers, header_value, header))
        return 0;
    return strtoull(header_value, NULL, 0);
}

bool is_chunked(struct phr_header *headers, size_t num_headers)
{
    char encoding[9];
    if (!get_header_value(headers, num_headers, encoding, "transfer-encoding"))
        return false;
    return (strcasecmp(encoding, "chunked") == 0);
}

bool read_chunked(HTTP_INFO *httpinfo, struct download *buffer, size_t start_pos)
{
    struct phr_chunked_decoder decoder = {0};
    size_t rsize, capacity = 4096;
    ssize_t pret;
    int ret;
    decoder.consume_trailer = true;
#ifdef DEBUG_NETWORK
    gprintf("Data is chunked\n");
#endif
    do
    {
        if (buffer->show_progress)
        {
            if (ProgressCanceled())
                return false;
            ShowProgress(start_pos, capacity); // Unknown size for chunked transfers
        }
        if (start_pos == capacity)
        {
#ifdef DEBUG_NETWORK
            gprintf("Increased buffer size\n");
#endif
            capacity *= 2;
            buffer->data = MEM2_realloc(buffer->data, capacity);
            if (!buffer->data) // A custom theme is using too much memory
            {
#ifdef DEBUG_NETWORK
                gprintf("Out of memory!\n");
#endif
                errno = ENOMEM;
                return false;
            }
        }
        if ((ret = https_read(httpinfo, &buffer->data[start_pos], capacity - start_pos, false)) < 1)
            return false;
        rsize = ret;
        pret = phr_decode_chunked(&decoder, &buffer->data[start_pos], &rsize);
        if (pret == -1)
        {
#ifdef DEBUG_NETWORK
            gprintf("Parse error\n");
#endif
            return false;
        }
        start_pos += rsize;
    } while (pret == -2);
    buffer->size = start_pos;
    buffer->data = MEM2_realloc(buffer->data, buffer->size);
    return true;
}

bool read_all(HTTP_INFO *httpinfo, struct download *buffer, size_t start_pos)
{
    size_t capacity = 4096;
    int ret;
#ifdef DEBUG_NETWORK
    gprintf("Data is not chunked\n");
#endif
    while (true)
    {
        if (buffer->show_progress)
        {
            if (ProgressCanceled())
                return false;
            ShowProgress(start_pos, buffer->content_length);
        }
        if (start_pos == capacity)
        {
#ifdef DEBUG_NETWORK
            gprintf("Increased buffer size\n");
#endif
            capacity *= 2;
            buffer->data = MEM2_realloc(buffer->data, capacity);
            if (!buffer->data) // A custom theme is using too much memory
            {
#ifdef DEBUG_NETWORK
                gprintf("Out of memory!\n");
#endif
                errno = ENOMEM;
                return false;
            }
        }
        if ((ret = https_read(httpinfo, &buffer->data[start_pos], capacity - start_pos, false)) == 0)
            break;
        if (ret < 0)
            return false;
        start_pos += ret;
    };
    buffer->size = start_pos;
    buffer->data = MEM2_realloc(buffer->data, buffer->size);
    return (buffer->content_length > 0 && buffer->content_length == start_pos);
}

bool get_response(HTTP_INFO *httpinfo, HTTP_RESPONSE *resp, bool proxy)
{
    int rret, minor_version;
    size_t msg_len, prevbuflen;
    const char *msg;

    while (true)
    {
        if ((rret = https_read(httpinfo, &resp->data[resp->buflen], 1, proxy)) < 1)
            return false;
        prevbuflen = resp->buflen;
        resp->buflen += rret;
        // Parse the response
        resp->num_headers = sizeof(resp->headers) / sizeof(resp->headers[0]);
        if ((resp->pret = phr_parse_response(resp->data, resp->buflen, &minor_version, &resp->status, &msg, &msg_len,
                                             resp->headers, &resp->num_headers, prevbuflen)) > 0)
            return true;
        else if (resp->pret == -1)
        {
#ifdef DEBUG_NETWORK
            gprintf("pret error %i\n", resp->pret);
#endif
            return false;
        }
        if (resp->buflen == sizeof(resp->data))
        {
#ifdef DEBUG_NETWORK
            gprintf("buflen error %lu\n", (unsigned long)resp->buflen);
#endif
            return false;
        }
    }
    return false;
}

bool check_ip(char *str)
{
    int partA, partB, partC, partD;
    char extra;
    // We avoid using regex because it increases the file size
    return (sscanf(str, "%d.%d.%d.%d%c", &partA, &partB, &partC, &partD, &extra) == 4);
}

bool connect_proxy(HTTP_INFO *httpinfo, char *host, char *username, char *password)
{
    HTTP_RESPONSE response = {0};
    char request[500];
    char credentials[66];
    char *auth;
    int len;
    if (username && password)
    {
        if (!snprintf(credentials, sizeof(credentials), "%s:%s", username, password))
            return false;
        if (!(auth = base64(credentials, strlen(credentials), &len)))
            return false;
        len = snprintf(request, sizeof(request),
                       "CONNECT %s:%i HTTP/1.1\r\nProxy-Authorization: Basic %s\r\nUser-Agent: curl/7.55.1\r\n\r\n",
                       host, httpinfo->use_https ? 443 : 80, auth);
        MEM2_free(auth);
    }
    else
        len = snprintf(request, sizeof(request),
                       "CONNECT %s:%i HTTP/1.1\r\nUser-Agent: curl/7.55.1\r\n\r\n",
                       host, httpinfo->use_https ? 443 : 80);
    if (len > 0 && https_write(httpinfo, request, len, true) != len)
        return false;
    if (get_response(httpinfo, &response, true))
    {
        if (response.status == 200)
            return true;
    }
    return false;
}

int connect(char *host, u16 port)
{
    struct sockaddr_in sin;
    s32 sock, ret;
    u32 ipaddress;
    u64 time;
#ifdef DEBUG_NETWORK
    gprintf("Connecting to %s", host);
#endif
    if ((ipaddress = check_ip(host) ? inet_addr(host) : getipbynamecached(host)) == 0)
        return -EFAULT;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = ipaddress;
#ifdef DEBUG_NETWORK
    if (!check_ip(host))
        gprintf(" (%s)", inet_ntoa(sin.sin_addr));
#endif
    if ((sock = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0)
        return sock;
    net_fcntl(sock, F_SETFL, 4);
    time = gettime();
    while (ticks_to_millisecs(diff_ticks(time, gettime())) < CONNECT_TIMEOUT)
    {
        if ((ret = net_connect(sock, (struct sockaddr *)&sin, sizeof(sin))) < 0)
        {
            if (ret == -EISCONN)
                return sock;
            if (ret == -EINPROGRESS || ret == -EALREADY)
            {
                usleep(10000);
                continue;
            }
            net_close(sock);
            return ret;
        }
    }
    net_close(sock);
    return -ETIMEDOUT;
}

void downloadfile(const char *url, struct download *buffer)
{
    HTTP_INFO httpinfo = {0};
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
        return;
    if (!path)
        return;
    // Get the host
    int domainlength = path - url - 7 - httpinfo.use_https;
    char host[domainlength + 1];
    strlcpy(host, url + 7 + httpinfo.use_https, domainlength + 1);
    // Start connecting
    if (getProxyAddress() && getProxyPort() > 0)
        httpinfo.sock = connect(getProxyAddress(), getProxyPort());
    else
        httpinfo.sock = connect(host, httpinfo.use_https ? 443 : 80);

    if (httpinfo.sock < 0)
    {
#ifdef DEBUG_NETWORK
        if (httpinfo.sock == -ETIMEDOUT)
            gprintf("\nFailed to connect (timed out)\n");
        else
            gprintf("\nFailed to connect (%i)\n", httpinfo.sock);
#endif
        return;
    }
#ifdef DEBUG_NETWORK
    gprintf("\nConnected\n");
#endif
    // Connect to a web proxy
    if (getProxyAddress() && getProxyPort() > 0)
    {
        if (!connect_proxy(&httpinfo, host, getProxyUsername(), getProxyPassword()))
        {
#ifdef DEBUG_NETWORK
            gprintf("Failed to connect to proxy (%s:%i)\n", getProxyAddress(), getProxyPort());
#endif
            https_close(&httpinfo);
            return;
        }
        session = NULL; // Resume doesn't work with a proxy
#ifdef DEBUG_NETWORK
        gprintf("Proxy is ready to receive\n");
#endif
    }
    // Setup for HTTPS if it's necessary
    if (httpinfo.use_https)
    {
        // Create a new SSL context
        // TLS 1.2 is slightly faster on Wii
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
        // Custom I/O is essential due to how libogc handles errors
        wolfSSL_SetIOSend(httpinfo.ctx, send_callback);
        wolfSSL_SetIORecv(httpinfo.ctx, recv_callback);
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
        if (session && wolfSSL_set_session(httpinfo.ssl, session) != SSL_SUCCESS)
        {
#ifdef DEBUG_NETWORK
            gprintf("Failed to set session (session timed out?)\n");
#endif
            session = NULL;
        }
        // Initiate a handshake
        u64 time = gettime();
        while (true)
        {
            if (ticks_to_millisecs(diff_ticks(time, gettime())) > CONNECT_TIMEOUT)
            {
#ifdef DEBUG_NETWORK
                gprintf("SSL handshake failed\n");
#endif
                https_close(&httpinfo);
                return;
            }
            if (wolfSSL_connect(httpinfo.ssl) == SSL_SUCCESS)
                break;
            usleep(10000);
        }
        // Check if we resumed successfully
        if (session && !wolfSSL_session_reused(httpinfo.ssl))
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
    // Save the session
    if (httpinfo.use_https)
        session = wolfSSL_get_session(httpinfo.ssl);
    // Send our request
    char request[2300];
    int ret, len;
    len = snprintf(request, sizeof(request),
                   "%s %s HTTP/1.1\r\n"
                   "Host: %s\r\n"
                   "User-Agent: USBLoaderGX/%s\r\n"
                   "Connection: close\r\n"
                   "Pragma: no-cache\r\n"
                   "Cache-Control: no-cache\r\n\r\n",
                   buffer->gametdbcheck ? "HEAD" : "GET", path, host, GetRev());
    if ((ret = https_write(&httpinfo, request, len, false)) != len)
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
    HTTP_RESPONSE response = {0};
    if (!get_response(&httpinfo, &response, false))
    {
        https_close(&httpinfo);
        return;
    }
    // The website wants to redirect us
    if (response.status == 301 || response.status == 302)
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
        char location[2049];
        if (!get_header_value(response.headers, response.num_headers, location, "location"))
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
        buffer->gametdbcheck = get_header_value_int(response.headers, response.num_headers, "x-gametdb-timestamp");
        https_close(&httpinfo);
        return;
    }
    // We got what we wanted
    if (response.status == 200)
    {
        buffer->data = MEM2_alloc(4096);
        memcpy(buffer->data, &response.data[response.pret], response.buflen - response.pret);
        // Determine how to read the data
        bool dl_valid;
        if (is_chunked(response.headers, response.num_headers))
            dl_valid = read_chunked(&httpinfo, buffer, response.buflen - response.pret);
        else
        {
            buffer->content_length = get_header_value_int(response.headers, response.num_headers, "content-length");
            dl_valid = read_all(&httpinfo, buffer, response.buflen - response.pret);
        }
        // Check if the download is incomplete
        if (!dl_valid || buffer->size < 1)
        {
            buffer->size = 0;
            MEM2_free(buffer->data);
#ifdef DEBUG_NETWORK
            gprintf("Removed incomplete download\n");
#endif
            https_close(&httpinfo);
            return;
        }
        // Finished
        https_close(&httpinfo);
#ifdef DEBUG_NETWORK
        gprintf("Download size: %llu\n", (long long)buffer->size);
        gprintf("------------- HEADERS -------------\n");
        for (size_t i = 0; i != response.num_headers; ++i)
            gprintf("%.*s: %.*s\n", (int)response.headers[i].name_len, response.headers[i].name,
                    (int)response.headers[i].value_len, response.headers[i].value);
        gprintf("------------ COMPLETED ------------\n");
#endif
        return;
    }
    // Close on all other status codes
#ifdef DEBUG_NETWORK
    gprintf("Status code: %i - %s\n", response.status, url);
#endif
    https_close(&httpinfo);
}
