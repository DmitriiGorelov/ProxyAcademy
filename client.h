#ifndef CLIENT_H
#define CLIENT_H

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
#include <QStringList>

#include "definitions.h"
#include "types.h"
#include "id.h"

class Client : public QObject, public QRunnable, uid
{
    Q_OBJECT
public:
    explicit Client(QObject* parent, qintptr _handle);

    void setSemaphore(QSemaphore* sema);

    QString name();
    void name(const QString& value);

signals:
    void closed();
    void dataRead(QByteArray data);
    void canWrite();
    void someError();
    void firstRequest(QByteArray data);
    void infoSizeIn(qintptr handle, unsigned long long size);

public slots:
    void dataWrite(QByteArray data);
    void stop();

private slots:
    void soketError(QAbstractSocket::SocketError socketError);
    void socketReadyRead();
    void socketDisconnected();

private: signals:
    void stopped();

public:
    void run() final;

private:
    qintptr handle;
    QTcpSocket* socket;
    QAtomicInt canRun;
    QList<QByteArray> dataOut;
    QMutex mutexOut;
    QSemaphore* semaphore;
    QString m_name;
    QMutex m_mutexName;
    QEventLoop* eventLoop;
    QMutex m_mutexEventLoop;
    tDataSize m_sizeIn;
};

#endif // CLIENT_H
