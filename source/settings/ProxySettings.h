#ifndef _PROXYSETTINGS_
#define _PROXYSETTINGS_

#ifdef __cplusplus
extern "C"
{
#endif
    void getProxyInfo();
    char *getProxyAddress();
    u16 getProxyPort();
    char *getProxyUsername();
    char *getProxyPassword();
#ifdef __cplusplus
}
#endif

#endif /* _PROXYSETTINGS_ */
