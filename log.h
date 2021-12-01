#ifndef LOG_H
#define LOG_H

#include <QDebug>
#include <QDataStream>
#include <QMutex>
#include <QMutexLocker>
#include <QString>

class QDataStreamMy
{
public:
    QDataStreamMy()
    {

    }

    template<typename T>
    QDataStreamMy& operator<<(const T& data)
    {
        /*static QMutex mutex;
        static QByteArray dev;
        static QDataStream logstrm(&dev, QIODevice::WriteOnly);

        QMutexLocker lock(&mutex);
        dev.clear();
        logstrm << data;*/

        return *this;
    }
};

inline QDataStreamMy& logstream()
{

    static QDataStreamMy logstrm;


    return logstrm;
}

inline QString AccumPrintout(const QStringList& sent, QString space="     ")
{
    QString s("");
    s.clear();
    for (auto& it : sent)
    {
        s+="\r\n"+space+it;
    }
    return s;
}

#define SHOW_DATA_CLIENT_N 0
#define SHOW_DATA_ENDPOINT_N 0

#define IF_LOG_CLIENT_ERR 0
#define IF_LOG_CLIENT_ALL 0
#define IF_LOG_ENDPOINT_ERR 0
#define IF_LOG_ENDPOINT_ALL 0
#define IF_LOG_CONNECT 0
#define IF_LOG_HEAD 0
#define IF_LOG_GET 0
#define IF_LOG_POST 0
#define IF_LOG_OTHER 0

#if IF_LOG_CLIENT_ERR
#define LOG_CLIENT_ERR qInfo()
#else
#define LOG_CLIENT_ERR logstream()
#endif

#if IF_LOG_CLIENT_ALL
#define LOG_CLIENT_RUN qInfo()
#define LOG_CLIENT_IN qInfo()
#define LOG_CLIENT_CLOSE qInfo()
#else
#define LOG_CLIENT_RUN logstream()
#define LOG_CLIENT_IN logstream()
#define LOG_CLIENT_CLOSE logstream()
#endif

#if IF_LOG_ENDPOINT_ERR
#define LOG_ENDPOINT_ERR qInfo()
#else
#define LOG_ENDPOINT_ERR logstream()
#endif

#if IF_LOG_ENDPOINT_ALL
#define LOG_ENDPOINT_RUN qInfo()
#define LOG_ENDPOINT_IN qInfo()
#define LOG_ENDPOINT_CLOSE qInfo()
#else
#define LOG_ENDPOINT_RUN  logstream()
#define LOG_ENDPOINT_IN logstream()
#define LOG_ENDPOINT_CLOSE logstream()
#endif

#if IF_LOG_CONNECT
#define LOG_CONNECT qInfo().noquote()
#else
#define LOG_CONNECT logstream()
#endif

#if IF_LOG_HEAD
#define LOG_HEAD qInfo().noquote()
#else
#define LOG_HEAD logstream()
#endif

#if IF_LOG_GET
#define LOG_GET qInfo().noquote()
#else
#define LOG_GET logstream()
#endif

#if IF_LOG_POST
#define LOG_POST qInfo().noquote()
#else
#define LOG_POST logstream()
#endif

#if IF_LOG_OTHER
#define LOG_OTHER qInfo().noquote()
#else
#define LOG_OTHER logstream()
#endif

#endif // LOG_H

