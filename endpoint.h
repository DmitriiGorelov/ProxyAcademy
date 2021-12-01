#ifndef ENDPOINT_H
#define ENDPOINT_H

#include <QObject>
#include <QDebug>
#include <QRunnable>
#include <QThread>
#include <QTcpSocket>
#include <QEventLoop>
#include <QList>
#include <QMutex>
#include <QByteArray>
#include <QSemaphore>
#include <QAtomicInt>
#include <QAuthenticator>
#include <QMap>
#include <QTimer>

#include "connectioninfo.h"
#include "types.h"
#include "definitions.h"
#include "id.h"

typedef std::function<void(connectioninfo*)> fCallConnectionInit;

struct epSettings
{
    int connTimeout;
    fCallConnectionInit callConnectionInit;
};

class Endpoint : public QObject, public QRunnable, uid
{
    Q_OBJECT
public:
    explicit Endpoint(QObject *parent, qintptr _handle, epSettings _settings);
    void setSemaphore(QSemaphore* sema);

signals:
    void closed();
    void dataRead(QByteArray data);
    void canWrite();
    void someError();    
    void infoSizeOut(qintptr handle, unsigned long long size);    

public slots:
    void dataWrite(QByteArray data);
    void stop();
    void firstRequest(QByteArray data);
    void timeout();

private slots:
    void soketError(QAbstractSocket::SocketError socketError);
    void socketReadyRead();
    void socketDisconnected();

private: signals:
    void stopped();

public:
    void run() final;

private:
    QString name();
    void name(QString value);

    void restartWatchDog();
    void stopWatchDog();
private:
    qintptr handle;
    QTcpSocket* socket;
    QAtomicInt canRun;
    QList<QByteArray> dataOut;
    QMutex mutexOut;
    QSemaphore* semaphore;
    QEventLoop* eventLoop;
    QMutex m_mutexEventLoop;
    QAtomicInt CounterDataIn;
    QAtomicInt CounterDataOut;
    QAtomicInt initSocket;
    std::once_flag flag;

    QString m_Name;
    QMutex m_mutexName;
    QString m_Address;
    int m_Port;
    QString m_Version;
    int m_Method;    
    tDataSize m_sizeOut;
    QString m_Auth;

    QByteArray m_FirstGetRequest;    
    QMap<QString, QString> first_request;

    QTimer* m_wdTimer;
    QMutex m_mutexwdTimer;

    epSettings m_Settings;
};

#endif // ENDPOINT_H
