#ifndef CONNECTIONINFO_H
#define CONNECTIONINFO_H

#include <QString>

#include "definitions.h"

struct connectioninfo
{
    public:
        connectioninfo()
            : m_Handle(0)
            , m_name("")
#if PROXYQT_TRANSPARENT
            , m_Address(RemoteProxyIP)
            , m_Port(RemoteProxyPort)
#else
            , m_Address("")
            , m_Port(0)
#endif
            , m_Version("HTTP/1.1")
            , m_Method(METHOD_CONNECT)
            , m_sizeIn(0)
            , m_sizeOut(0)
            , m_User(-1)
            , m_Auth("")
            , m_Blocked(-2)
            , m_Peer()
        {

        }

        QString methodName()
        {
            switch (method())
            {
                case METHOD_CONNECT:
                {
                    return "CONNECT";
                    break;
                }
                case METHOD_GET:
                {
                    return "GET";
                    break;
                }
                case METHOD_POST:
                {
                    return "POST";
                    break;
                }
                case METHOD_HEAD:
                {
                    return "HEAD";
                    break;
                }
                default:
                    break;
            }
        }

        QString name() {return m_name;}
        void name(QString value) {m_name=value;}

        QString address() {return m_Address;}
        void address(QString value) {m_Address=value;}

        unsigned short port() {return m_Port;}
        void port(unsigned short value) {m_Port=value;}

        int method() {return m_Method;}
        void method(int value) { m_Method=value;}

        QString version() {return m_Version;}
        void version(QString value) {m_Version=value;}

        qintptr handle() { return m_Handle;}
        void handle(qintptr value) {m_Handle=value;}

        unsigned long long sizeIn() {return m_sizeIn;}
        void sizeIn(unsigned long long value) {m_sizeIn = value;}

        unsigned long long sizeOut() {return m_sizeOut;}
        void sizeOut(unsigned long long value) {m_sizeOut = value;}

        QString auth() {return m_Auth;}
        void auth(QString value) {m_Auth=value;}

        int user() {return m_User;}
        void user(int value) { m_User=value;}

        int blocked() {return m_Blocked;}
        void blocked(int value) {m_Blocked=value;}

        QString peer() {return m_Peer;}
        void peer(QString value) { m_Peer=value;}
    private:
        qintptr m_Handle;
        QString m_name;
        QString m_Address;
        unsigned short m_Port;
        QString m_Version;
        int m_Method;
        unsigned long long m_sizeIn;
        unsigned long long m_sizeOut;
        int m_User;
        QString m_Auth;
        int m_Blocked;
        QString m_Peer;
};

#endif // CONNECTIONINFO_H
