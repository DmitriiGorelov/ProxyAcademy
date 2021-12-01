#include "client.h"
#include "log.h"
#include "mymutexlocker.h"
#include <QDataStream>
#include <QHostAddress>

Client::Client(QObject* parent, qintptr _handle)
    : QObject(parent)
    , QRunnable()
    , uid()
    , handle(0)
    , socket()
    , canRun()
    , dataOut()
    , mutexOut()
    , semaphore(nullptr)
    , m_name()
    , m_mutexName()
    , eventLoop(nullptr)
    , m_mutexEventLoop()
    , m_sizeIn(0)
{
    handle = _handle;
}

void Client::setSemaphore(QSemaphore* sema)
{
    semaphore=sema;
}

QString Client::name()
{
    MyMutexLocker lock(&m_mutexName, ids()+"mutexName", PLACE);
    return m_name;
}

void Client::name(const QString& value)
{
    MyMutexLocker lock(&m_mutexName, ids()+"mutexName", PLACE);
    m_name=value;
}

void Client::dataWrite(QByteArray data)
{
    {
        MyMutexLocker lock(&mutexOut, ids() + "mutexOut", PLACE);
        dataOut.append(data);
    }

    MyMutexLocker lock(&m_mutexEventLoop, ids()+"mutexEventLoop", PLACE);
    if (eventLoop && eventLoop->isRunning())
        eventLoop->quit();
}

void Client::stop()
{
    canRun=false;
    MyMutexLocker lock(&m_mutexEventLoop, ids() + "mutexEventLoop", PLACE);

    if (semaphore)
    {
        semaphore->release(1);
    }

    if (eventLoop && eventLoop->isRunning())
        eventLoop->quit();
}

void Client::run()
{
        canRun=true;

        LOG_CLIENT_RUN << this << " run " << QThread::currentThread();

        if (semaphore)
        {
            semaphore->acquire(1);
        }

        socket = new QTcpSocket(nullptr);

        if(!socket->setSocketDescriptor(handle))
        {
            LOG_CLIENT_ERR << socket->errorString();
            delete socket;
            return;
        }

        /*qDebug() << ids() << " " << socket->peerAddress().toString();
        qDebug() << ids() << " " << socket->peerName();
        qDebug() << ids() << " " << socket->peerPort();*/

        eventLoop = new QEventLoop();

        connect(socket, &QTcpSocket::readyRead,this,&Client::socketReadyRead,Qt::ConnectionType::DirectConnection);
        connect(socket, &QTcpSocket::disconnected,this,&Client::socketDisconnected,Qt::ConnectionType::DirectConnection);
        connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(soketError(QAbstractSocket::SocketError)),Qt::ConnectionType::DirectConnection);

        while (canRun)
        {
            eventLoop->exec();

            if (canRun)
            {
                MyMutexLocker lock(&mutexOut, ids() + "mutexOut", PLACE);
                while (dataOut.size()>0)
                {
                    QByteArray data= dataOut.front();
                    dataOut.pop_front();
                    if (socket->state()==QTcpSocket::SocketState::ConnectedState)
                    {
                        //if (!id().isEmpty())
                        LOG_CLIENT_IN << "IN " << id() << " bytes: " << data.size();

                        m_sizeIn+=data.size();
                        if (socket->write(data))
                        {
                            if (socket->waitForBytesWritten())
                            {

                            }
                            else
                            {
                                LOG_CLIENT_ERR << socket->error();
                                LOG_CLIENT_ERR << socket->errorString();
                                canRun=false;
                                emit someError();
                            }
                        }
                        else
                        {
                            LOG_CLIENT_ERR << socket->error();
                            LOG_CLIENT_ERR << socket->errorString();
                            canRun=false;
                            emit someError();
                            break;
                        }
                    }
                }
                emit infoSizeIn(handle, m_sizeIn);

            }
        }


        socket->close();
        socket->deleteLater();

        {
            MyMutexLocker lock(&m_mutexEventLoop, ids() + "mutexEventLoop", PLACE);
            eventLoop->deleteLater();
            eventLoop=nullptr;
        }

        emit closed();

        LOG_CLIENT_CLOSE << "Client" << id() << " done " << QThread::currentThread();
}

void Client::soketError(QAbstractSocket::SocketError socketError)
{
    LOG_CLIENT_ERR << "Client Error:" << socketError << " ";

    canRun=false;

    MyMutexLocker lock(&m_mutexEventLoop, ids() + "mutexEventLoop", PLACE);
    if (eventLoop && eventLoop->isRunning())
        eventLoop->quit();
}

void Client::socketDisconnected()
{
    LOG_CLIENT_ERR << "Client Disconnected:" << id() << " ";

    canRun=false;

    MyMutexLocker lock(&m_mutexEventLoop, ids() + "mutexEventLoop", PLACE);
    if (eventLoop && eventLoop->isRunning())
        eventLoop->quit();
}

void Client::socketReadyRead()
{    
    if (socket->state()==QTcpSocket::SocketState::ConnectedState && canRun)
    {
        QByteArray data=socket->readAll();

        emit dataRead(data);
    }
}
