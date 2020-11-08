#include <ogcsys.h>
#include <ogc/isfs.h>
#include <string.h>

#include "ProxySettings.h"
#include "settings/CSettings.h"

#define ALIGN32(x) (((x) + 31) & ~31)

bool proxy_enabled;
bool proxy_creds_enabled;
char proxy_address[256];
u16 proxy_port;
char proxy_username[33];
char proxy_password[33];

void getProxyInfo()
{
    char *buffer;
    int fd = ISFS_Open("/shared2/sys/net/02/config.dat", ISFS_OPEN_READ);
    if (fd >= 0)
    {
        fstats stats ATTRIBUTE_ALIGN(32);
        if (ISFS_GetFileStats(fd, &stats) >= 0)
        {
            if (stats.file_length == 7004)
            {
                buffer = (char *)MEM2_alloc(ALIGN32(stats.file_length));
                if (buffer)
                {
                    if (ISFS_Read(fd, buffer, stats.file_length) == 7004)
                    {
                        proxy_enabled = buffer[44];
                        proxy_creds_enabled = buffer[45];
                        strncpy(proxy_address, buffer + 48, sizeof(proxy_address) - 1);
                        proxy_port = ((buffer[304] & 0xFF) << 8) | (buffer[305] & 0xFF);
                        strncpy(proxy_username, buffer + 306, sizeof(proxy_username) - 1);
                        strncpy(proxy_password, buffer + 338, sizeof(proxy_password) - 1);
                    }
                }
                MEM2_free(buffer);
            }
        }
        ISFS_Close(fd);
    }
}

char *getProxyAddress()
{
    if (Settings.ProxyUseSystem)
        return proxy_enabled ? proxy_address : NULL;
    return (strlen(Settings.ProxyAddress) > 6) ? Settings.ProxyAddress : NULL;
}

u16 getProxyPort()
{
    if (Settings.ProxyUseSystem)
        return proxy_enabled ? proxy_port : 0;
    return Settings.ProxyPort;
}

char *getProxyUsername()
{
    if (Settings.ProxyUseSystem)
        return proxy_enabled && proxy_creds_enabled ? proxy_username : NULL;
    return (strlen(Settings.ProxyUsername) > 0) ? Settings.ProxyUsername : NULL;
}

char *getProxyPassword()
{
    if (Settings.ProxyUseSystem)
        return proxy_enabled && proxy_creds_enabled ? proxy_password : NULL;
    return (strlen(Settings.ProxyPassword) > 0) ? Settings.ProxyPassword : NULL;
}
