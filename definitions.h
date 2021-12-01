#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define RemoteProxyIP "127.0.0.8"
#define RemoteProxyPort 9090
#define clientPort 9091
#define snifferPort 2345

#define THREADS_PER_USER 40
#define CONN_LIVE_TIMEOUT 1
#define CONN_ONSCREEN_TIMEOUT 5

#define PLACE QString(__FILE__)+" "+QString::number(__LINE__)

#define PROXYQT_TRANSPARENT 0
//#define USE_AUTH 1
//#define AUTH_USER "user"
//#define AUTH_PASS "user"

#define METHOD_CONNECT 1
#define METHOD_GET 0
#define METHOD_HEAD 2
#define METHOD_OPTIONS 3
#define METHOD_POST 4

#define REQ_TYPE "Type"
#define REQ_PATH "Path"
#define REQ_VERSION "Version"
#define REQ_AUTH "Proxy-Authorization"

#define justComeToInit -1
#define allowedOK 0
#define blockedAuth 1
#define blockedSize 2
#define blockedLink 3
#define blockedTime 4
#define blockedUserDisabled 5


#endif // DEFINITIONS_H

