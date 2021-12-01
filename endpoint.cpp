#include "endpoint.h"
#include "mymutexlocker.h"
#include <QMutex>
#include <QJsonDocument>

#include "client.h"
#include "log.h"

Endpoint::Endpoint(QObject *parent, qintptr _handle, epSettings _settings)
    : QObject(parent)
    , QRunnable()
    , uid()
    , handle(_handle)
    , socket(nullptr)
    , canRun(true)
    , dataOut()
    , mutexOut()
    , semaphore(nullptr)
    , eventLoop(nullptr)
    , m_mutexEventLoop()
    , CounterDataIn(0)
    , CounterDataOut(0)
    , initSocket()
    , flag()

    , m_Name("")
    , m_mutexName()
#if PROXYQT_TRANSPARENT
    , m_Address(RemoteProxyIP)
    , m_Port(RemoteProxyPort)
#else
    , m_Address("")
    , m_Port(0)
#endif
    , m_Version("HTTP/1.1")
    , m_Method(METHOD_CONNECT)    
    , m_sizeOut(0)
    , m_Auth("")

    , m_FirstGetRequest()
    , first_request()
    , m_wdTimer(nullptr)
    , m_mutexwdTimer()
    , m_Settings(_settings)
{
}

void Endpoint::setSemaphore(QSemaphore* sema)
{
    semaphore=sema;
}

void Endpoint::dataWrite(QByteArray data)
{    
    {
        MyMutexLocker lock(&mutexOut, ids() + "mutexOut", PLACE);

        if (CounterDataOut==0)
        {
            firstRequest(data);
        }
        else
        {
            dataOut.append(data);
        }
        CounterDataOut++;
    }


    MyMutexLocker lock(&m_mutexEventLoop, ids() + "mutexEventLoop", PLACE);
    if (eventLoop && eventLoop->isRunning())
        eventLoop->quit();

    //emit canWrite();
}

void Endpoint::timeout()
{
    stop();
}

void Endpoint::stop()
{    
    canRun=false;
    MyMutexLocker lock(&m_mutexEventLoop, ids() + "mutexEventLoop", PLACE);

    if (semaphore)
    {
        semaphore->release(1);
    }

    if (eventLoop && eventLoop->isRunning())
        eventLoop->quit();
    //emit stopped();
}

QString Endpoint::name()
{
    MyMutexLocker lock(&m_mutexName,  ids() + "mutexName", PLACE);
    return m_Name;
}

void Endpoint::name(QString value)
{
    MyMutexLocker lock(&m_mutexName,  ids() + "mutexName", PLACE);
    m_Name = value;
}

void Endpoint::firstRequest(QByteArray data)
{
    if (!canRun)
        return;

    QString s(data);

    QStringList sentences = s.split("\r\n", QString::SkipEmptyParts);
    if (sentences.size()>0)
    {
        QStringList words0 = sentences[0].split(' ', QString::SkipEmptyParts);
        if (words0.size() > 2)
        {
            name(words0[1]);

            if (words0[0] == "CONNECT")
            {
                LOG_CONNECT << "CONNECT: (" << words0[1]<< ")" << AccumPrintout(sentences);

                m_Method=(METHOD_CONNECT);
                QStringList destinationSentence = words0[1].split(":");
                if (destinationSentence.size()>1)
                {
                    m_Address=(destinationSentence[0]);
                    m_Port=(destinationSentence[1].toUShort());
                }

                m_Version=(words0[2]);
            }
            else if (words0[0] == "GET")
            {
                LOG_GET << "GET: (" << words0[1]<< ")" << AccumPrintout(sentences);

                m_Method=(METHOD_GET);
                {
                    dataOut.insert(0, data);
                }

                int a(-1);
                for (auto& it : sentences)
                {
                    a = it.indexOf("Host: ");
                    if (a>-1)
                    {
                        m_Address=(it.mid(6,static_cast<int>(a)-6));
                        break;
                    }
                }
                if (a<0)
                {
                    LOG_ENDPOINT_ERR << "HOST EMPTY!";
                    canRun=false;
                    return;
                }
                //m_Address=words0[1];
                m_Port=(80);

                m_Version=(words0[2]);
            }
            else if (words0[0] == "POST")
            {
                LOG_POST << "POST: (" << words0[1]<< ")" << AccumPrintout(sentences);

                m_Method=(METHOD_POST);
                {
                    dataOut.insert(0, data);
                }

                int a(-1);
                for (auto& it : sentences)
                {
                    a = it.indexOf("Host: ");
                    if (a>-1)
                    {
                        m_Address=(it.mid(6,static_cast<int>(a)-6));
                        break;
                    }
                }
                if (a<0)
                {
                    LOG_ENDPOINT_ERR << "HOST EMPTY!";
                    canRun=false;
                    return;
                }
                //m_Address=words0[1];
                m_Port=(80);

                m_Version=(words0[2]);
            }
            else if (words0[0] == "HEAD")
            {
                LOG_HEAD << "HEAD: (" << words0[1]<< ")" << AccumPrintout(sentences);

                m_Method=(METHOD_HEAD);
                {
                    dataOut.insert(0, data);
                }

                int a(-1);
                for (auto& it : sentences)
                {
                    a = it.indexOf("Host: ");
                    if (a>-1)
                    {
                        m_Address=(it.mid(6,static_cast<int>(a)-6));
                        break;
                    }
                }
                if (a<0)
                {
                    LOG_ENDPOINT_ERR << "HOST EMPTY!";
                    canRun=false;
                    return;
                }
                //m_Address=words0[1];
                m_Port=(80);

                m_Version=(words0[2]);
            }
            else
            {
                LOG_OTHER << "COMMAND UNKNOWN! (" << words0[1]<< ")" << AccumPrintout(sentences);
            }
        }
        else
        {
            LOG_ENDPOINT_ERR << "COMMAND EMPTY!";
            canRun=false;
            return;
        }

        QString key("");
        QString value("");

        for (int k=1; k < sentences.size(); k++) //!! 1
        {
            if(sentences[k].size() == 0)
                break;

            int posFirst = sentences[k].indexOf(":",0); //Look for separator ':'

            key = sentences[k].mid(0, posFirst);
            value = sentences[k].mid(posFirst + 2);

            if (key.isEmpty())
                break;
            first_request[key] = value;
        }
    }
    else
    {
        LOG_ENDPOINT_ERR << "COMMAND EMPTY!";
        canRun=false;
        return;
    }
}

void Endpoint::run()
{
        //canRun=true;
        CounterDataIn=0;
        initSocket=0;

        LOG_ENDPOINT_RUN << this << " run " << QThread::currentThread();

#if PROXYQT_TRANSPARENT
        if (m_Address.isEmpty())
        {
            LOG_ENDPOINT_ERR << "Address is empty!";
            canRun=false;
            return;
        }
        LOG_ENDPOINT_RUN << "Try connect to " << m_Address << ":" << m_Port;
        socket = new QTcpSocket(nullptr);
        socket->connectToHost(m_Address, m_Port);
        if (!socket->waitForConnected(60000))
        {
            LOG_ENDPOINT_ERR << "Cannot connect Endpoint " << m_Address << ":" << m_Port << " - " << socket->errorString();
            delete socket;
            canRun = false;
            return;
        }

        connect(socket, &QTcpSocket::readyRead,this,&Endpoint::socketReadyRead,Qt::ConnectionType::DirectConnection);
        connect(socket, &QTcpSocket::disconnected,this,&Endpoint::socketDisconnected,Qt::ConnectionType::DirectConnection);
        connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(soketError(QAbstractSocket::SocketError)));
#endif
        eventLoop = new QEventLoop();

        while (canRun)
        {
            if (semaphore)
            {
                semaphore->release(1);
            }

            eventLoop->exec();

            if (canRun)
            {
#if !PROXYQT_TRANSPARENT
                //std::call_once(flag,[&](){ // THIS IS ASYNC CALL!!
                if (!initSocket)
                {
                    initSocket++;

                    if (m_Address.isEmpty())
                    {
                        LOG_ENDPOINT_ERR << "Address is empty!";
                        canRun=false;
                        break;
                    }



                    if (m_Settings.callConnectionInit)
                    {
                        connectioninfo info;

                        if (first_request.find(REQ_AUTH)==first_request.end())
                        {
//#if USE_AUTH
//                            //auth failed
//                            LOG_ENDPOINT_ERR << "Proxy Authorization failed!!";
//                            canRun=false;
//                            QByteArray data = m_Version.toUtf8() + " 407 Proxy Authentication Required\r\nServer: Proxy \r\nProxy-Authenticate: Basic realm=\"ProxyAcademy Authorization\"\r\nConnection: Close\r\nProxy-Connection: Close\r\nContent-Length: 0\r\n\r\n";
//                            LOG_ENDPOINT_IN << "FIRST ANSWER: " << data;
//                            {
//                                emit dataRead(data);
//                            }
//                            eventLoop->exec(); // to wait for client action -> have to wait for the 407 telegram to go before terminating endpoint
//                            break;
//#endif
                            m_Auth="";
                        }
                        else
                        {
                            QStringList auth=first_request[REQ_AUTH].split(' ');
                            if (auth.size()>1)
                                m_Auth = auth[1];
                        }

                        info.handle(handle);
                        info.address(m_Address);
                        info.name(m_Name);
                        info.port(m_Port);
                        info.method(m_Method);
                        info.auth(m_Auth);


                        m_Settings.callConnectionInit(&info);
                        m_Settings.callConnectionInit=nullptr;

                        switch (info.blocked())
                        {
                            case allowedOK:
                            {
                                break;
                            }
                            case blockedAuth:
                            {
                                // auth failed
                                LOG_ENDPOINT_ERR << "Proxy Authorization failed!!";
                                canRun=false;

                                QByteArray data = m_Version.toUtf8() + " 407 Proxy Authentication Required\r\nServer: Proxy \r\nProxy-Authenticate: Basic realm=\"ProxyAcademy Authorization\"\r\nConnection: Close\r\nProxy-Connection: Close\r\nContent-Length: 0\r\n\r\n";
                                LOG_ENDPOINT_IN << "FIRST ANSWER: " << data;

                                {
                                    emit dataRead(data);
                                }
                                eventLoop->exec(); // to wait for client action

                                break;
                            }
                            case blockedSize:
                            case blockedLink:
                            case blockedTime:
                            case blockedUserDisabled:
                            default:
                            {
                                LOG_ENDPOINT_ERR << "Endpoint " << m_Address << ":" << m_Port << " - " << "BLOCKED!";
                                canRun = false;
                                break;
                            }
                        }
                    }


                    if (!canRun)
                        break;

                    LOG_ENDPOINT_RUN << "Try connect to " << m_Address << ":" << m_Port;
                    socket = new QTcpSocket(nullptr);
                    socket->connectToHost(m_Address, m_Port);
                    if (!socket->waitForConnected(60000))
                    {
                        LOG_ENDPOINT_ERR << "Cannot connect Endpoint " << m_Address << ":" << m_Port << " - " << socket->errorString();
                        //delete socket;
                        canRun = false;
                        break;
                    }

                    connect(socket, &QTcpSocket::readyRead,this,&Endpoint::socketReadyRead,Qt::ConnectionType::DirectConnection);
                    connect(socket, &QTcpSocket::disconnected,this,&Endpoint::socketDisconnected,Qt::ConnectionType::DirectConnection);
                    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(soketError(QAbstractSocket::SocketError)));

                    switch (m_Method)
                    {
                        case METHOD_CONNECT:
                        {
                            QByteArray data = m_Version.toUtf8() + " 200 OK\r\nProxy-agent: ProxyAcademy\r\n\r\n";
                            LOG_ENDPOINT_IN << "FIRST ANSWER: " << data;

                            emit dataRead(data);
                            break;
                        }
                        case METHOD_GET:
                        {
                            break;
                        }
                        case METHOD_POST:
                        {
                            break;
                        }
                        case METHOD_HEAD:
                        {
                            break;
                        }
                        default:
                            break;
                    }
                }
#endif                    
                {
                    MyMutexLocker lock(&mutexOut, ids() + "mutexOut", PLACE);
                    while (dataOut.size()>0)
                    {
                        QByteArray data= dataOut.front();
                        dataOut.pop_front();
                        if (socket->state()==QTcpSocket::SocketState::ConnectedState)
                        {
                            m_sizeOut+=data.size();
                            if (socket->write(data))
                            {
                                if (socket->waitForBytesWritten())
                                {

                                }
                                else
                                {
                                    LOG_ENDPOINT_ERR << socket->error();
                                    LOG_ENDPOINT_ERR << socket->errorString();
                                    canRun=false;
                                    emit someError();
                                    break;
                                }
                            }
                            else
                            {
                                LOG_ENDPOINT_ERR << socket->error();
                                LOG_ENDPOINT_ERR << socket->errorString();
                                canRun=false;
                                emit someError();
                                break;
                            }
                        }
                    }

                    restartWatchDog();
                    emit infoSizeOut(handle, m_sizeOut);

                }
            }
        }


        if (socket)
        {
            socket->close();
            socket->deleteLater();
        }
        {
            MyMutexLocker lock(&m_mutexEventLoop, ids() + "mutexEventLoop", PLACE);
            eventLoop->deleteLater();
            eventLoop=nullptr;
        }

        emit closed();

        LOG_ENDPOINT_RUN << "Endpoint " << name() << " done " << QThread::currentThread();
}

void Endpoint::soketError(QAbstractSocket::SocketError socketError)
{
    LOG_ENDPOINT_ERR << "Endpoint Error:" << socketError << " ";// << socket->errorString(); // socket can already be deleted.

    canRun=false;

    MyMutexLocker lock(&m_mutexEventLoop, ids() + "mutexEventLoop", PLACE);
    if (eventLoop && eventLoop->isRunning())
        eventLoop->quit();

    //emit someError();
}

void Endpoint::socketDisconnected()
{
    LOG_ENDPOINT_ERR << "Endpoint Disconnected ";// << socket->errorString(); // socket can already be deleted.

    canRun=false;

    MyMutexLocker lock(&m_mutexEventLoop, ids() + "mutexEventLoop", PLACE);
    if (eventLoop && eventLoop->isRunning())
        eventLoop->quit();

    //emit someError();
}

void Endpoint::restartWatchDog()
{
    stopWatchDog();

    MyMutexLocker lock(&m_mutexwdTimer,ids()+"m_mutexwdTimer",PLACE);

    m_wdTimer = new QTimer();
    connect(m_wdTimer, &QTimer::timeout, this, &Endpoint::timeout);
    m_wdTimer->setSingleShot(true);
    m_wdTimer->start(m_Settings.connTimeout*1000);
}

void Endpoint::stopWatchDog()
{
    MyMutexLocker lock(&m_mutexwdTimer,ids()+"m_mutexwdTimer",PLACE);

    if (m_wdTimer)
    {
        m_wdTimer->stop();
        delete m_wdTimer;
    }
}

void Endpoint::socketReadyRead()
{    
    if (socket->state()==QTcpSocket::SocketState::ConnectedState && canRun)
    {
        QByteArray data=socket->readAll();

        restartWatchDog();

        emit dataRead(data);        
    }
}
