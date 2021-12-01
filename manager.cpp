
#include "manager.h"

#include <QtConcurrent/QtConcurrent>
#include "mymutexlocker.h"
#include <QSharedPointer>
#include <QFutureWatcher>
#include <QNetworkProxy>
#include <QHostAddress>
#include <QtNetwork>

#include <functional>
#include <algorithm>

#include "eventloophelper.h"
#include "log.h"

#include "base64.h"

Manager* Manager::manager;

Manager::Manager(QObject *parent)
    : QTcpServer(parent)
    , connections()
    , m_connectionsMutex()
    , m_Running(false)
    , m_Users()
    , w_settings()
    , m_Port(clientPort)
    , m_ConnLiveTimeout(CONN_LIVE_TIMEOUT)
    , m_ConnOnScreenTimeout(CONN_ONSCREEN_TIMEOUT)
    , m_ThreadsPerUser(THREADS_PER_USER)
{
    qRegisterMetaType<QAbstractSocket::SocketError>();
    qRegisterMetaType<connectioninfo>("connectioninfo");

    ReadParameters();
}

void Manager::start()
{
    m_Running=true;

    QThreadPool::globalInstance()->setMaxThreadCount(m_Users.size()*ThreadsPerUser());

    qInfo() << this << " start " << QThread::currentThread();
    this->setProxy(QNetworkProxy::NoProxy);
    if(this->listen(QHostAddress::AnyIPv4, m_Port))
    {
        qInfo() << "Server started on " << m_Port;
    }
    else
    {
        qCritical() << this->errorString();
    }

    for (auto& it : m_Users)
    {
        it.open();
    }
}

void Manager::quit()
{
    m_Running=false;    

    this->close();
    while (this->isListening() )
    {
        _sleep(1);
    }
    qInfo() << "Server Stopped!";

    stopConnections();

    {
        MyMutexLocker lock(&m_connectionsMutex, "connectionsMutex", PLACE);
        for (auto& it : m_Users)
        {
            it.close();
        }
    }

    emit closed();

    WriteParameters();
}

void Manager::callConnectiontInit(connectioninfo* info)
{
    info->blocked(justComeToInit);

    MyMutexLocker lock(&m_connectionsMutex, "connectionsMutex", PLACE);
    auto it = connections.find(info->handle());
    if (it!=connections.end())
    {
        int c(-1);
        // here is the reason of failure in infoSizeIn.If data comes emmediatelly when open connection, we can emit infoSizeIn BEFORE we find out theuser ID in lines below.
        // Therefore, User is -1...
        // But PROBLEM: we call callEndPointSocketInit() BEFORE initialize Socket! HOW can any data can come BEFORE we finish here??

        info->user(-1);
        //QThread::msleep(1000);
        if (info->auth().isEmpty() && m_Users.size()>0)
        {
            c=0;
            LOG_ENDPOINT_IN << "Anonimous user! ";
            info->user(c);
        }
        else
        {
            for (auto& it : m_Users)
            {
                c++;
                QString s(it.name()+QString(":")+it.pass());
                QString authStr(base64_encode(reinterpret_cast<const unsigned char*>(s.toStdString().c_str()), s.toStdString().length()).c_str());

                if (info->auth() == authStr)
                {
                    LOG_ENDPOINT_IN << "Authorization successful! ";

                    info->user(c);

                    break;
                }
            }
        }

        if (info->user() < 0)
        {
            info->blocked(blockedAuth);
            return; // have to return here, because User does not give valid key for m_Users map
        }

        info->blocked(allowedOK);

        m_Users[info->user()].newConnection(info);

        it->info =* info;

        emit ConnectionUpdated(-1, *info);
    }    
}

void Manager::infoSizeIn(qintptr handle, unsigned long long size)
{
    MyMutexLocker lock(&m_connectionsMutex, "connectionsMutex", PLACE);
    auto it = connections.find(handle);
    if (it!=connections.end())
    {
        if (it.value().info.user()<0)
        {
            qInfo() << "BAD DAY";
            return;
        }

        it.value().info.sizeIn(size);
        if (!m_Users[it.value().info.user()].addSize(it.value().info.name(),size))
        {
            it->endpoint->stop();
        }

        emit ConnectionUpdated(infoIdxSizeIn, it.value().info);
    }
}

void Manager::infoSizeOut(qintptr handle, unsigned long long size)
{
    MyMutexLocker lock(&m_connectionsMutex, "connectionsMutex", PLACE);
    auto it = connections.find(handle);
    if (it!=connections.end())
    {
        it.value().info.sizeOut(size);

        emit ConnectionUpdated(infoIdxSizeOut, it.value().info);
    }
}

bool Manager::isRunning()
{
    return m_Running;
}

void Manager::newBridge(qintptr handle)
{
    if (!Manager::Instance()->isRunning())
    {
        return;
    }
    QSemaphore sema;

    epSettings settings;
    settings.connTimeout=Manager::Instance()->ConnTimeout();
    settings.callConnectionInit=std::bind(&Manager::callConnectiontInit, Manager::Instance(), std::placeholders::_1);
    QSharedPointer<Endpoint> endpoint(new Endpoint(nullptr, handle, settings));
    QSharedPointer<Client> client(new Client(nullptr, handle));

    connect(client.get(), &Client::dataRead, endpoint.get(), &Endpoint::dataWrite, Qt::ConnectionType::QueuedConnection);
    connect(endpoint.get(), &Endpoint::dataRead, client.get(), &Client::dataWrite, Qt::ConnectionType::QueuedConnection);

    connect(client.get(), &Client::closed, endpoint.get(), &Endpoint::stop, Qt::ConnectionType::QueuedConnection);
    connect(endpoint.get(), &Endpoint::closed, client.get(), &Client::stop, Qt::ConnectionType::QueuedConnection);

    connect(Manager::Instance(), &Manager::closed, endpoint.get(), &Endpoint::stop, Qt::ConnectionType::QueuedConnection);
    connect(Manager::Instance(), &Manager::closed, client.get(), &Client::stop, Qt::ConnectionType::QueuedConnection);

    endpoint->setSemaphore(&sema);
    client->setSemaphore(&sema);

    endpoint->setAutoDelete(false); //!!
    client->setAutoDelete(false);

    Manager::Instance()->addConnection(handle, endpoint, client);

    QThreadPool pool;
    pool.setMaxThreadCount(2);    

    EventLoopHelper* eventLoop = new EventLoopHelper(nullptr,2);

    connect(client.get(), &Client::closed,eventLoop,&EventLoopHelper::inc,Qt::ConnectionType::QueuedConnection);
    connect(endpoint.get(), &Endpoint::closed,eventLoop,&EventLoopHelper::inc,Qt::ConnectionType::QueuedConnection);

    connect(client.get(), &Client::infoSizeIn, Manager::Instance(), &Manager::infoSizeIn, Qt::ConnectionType::QueuedConnection);
    connect(endpoint.get(), &Endpoint::infoSizeOut, Manager::Instance(), &Manager::infoSizeOut, Qt::ConnectionType::QueuedConnection);

    pool.start(client.get());
    pool.start(endpoint.get());

    eventLoop->exec();

    pool.waitForDone();    

    eventLoop->deleteLater();
    eventLoop=nullptr;

    Manager::Instance()->removeConnection(handle);

    endpoint->deleteLater(); //
    client->deleteLater();

    endpoint=nullptr;
    client=nullptr;

    LOG_ENDPOINT_CLOSE << "Bridge Exit! ";
}

void Manager::incomingConnection(qintptr handle)
{
    //https://doc.qt.io/qt-5/qtnetwork-threadedfortuneserver-example.html
    QFuture<void> future = QtConcurrent::run(Manager::newBridge, handle);
}

void Manager::addConnection(qintptr handle, QSharedPointer<Endpoint> endpoint, QSharedPointer<Client> client)
{    
    MyMutexLocker lock(&m_connectionsMutex, "connectionsMutex", PLACE);
    connectioninfo info;
    info.handle(handle);
    connection connect(info,endpoint,client);
    connections[handle] = connect;
}

void Manager::removeConnection(qintptr handle)
{
    MyMutexLocker lock(&m_connectionsMutex, "connectionsMutex", PLACE);
    auto it = connections.find(handle);
    if (it!=connections.end())
    {
        it.value().endpoint->stop();
        connections.erase(it);

        if (m_ConnOnScreenTimeout>0)
        {
            emit endPointClosed(handle);

            QTimer::singleShot(m_ConnOnScreenTimeout*1000, this, [this, handle](){ emit endPointRemoved(handle); });
        }
        else
        {
            emit endPointRemoved(handle);
        }

    }
}

void Manager::stopConnections()
{
    {
        MyMutexLocker lock(&m_connectionsMutex, "connectionsMutex", PLACE);
        for (auto& it : connections)
        {
            it.endpoint->stop();
        }
    }
    {
        while(connections.size()>0)
        {
            _sleep(1);
        }
    }
}

TPort Manager::Port()
{
    return m_Port;
}

int Manager::ConnTimeout()
{
    return  m_ConnLiveTimeout;
}

int Manager::ThreadsPerUser()
{
    return m_ThreadsPerUser;
}

void Manager::ShowSettingsDialog()
{
    w_settings.Port(m_Port);
    w_settings.ConnLiveTimeout(m_ConnLiveTimeout);
    w_settings.Threads(m_ThreadsPerUser);
    w_settings.ConnOnScreenTimeout(m_ConnOnScreenTimeout);
    w_settings.setUsers(m_Users);

    int res = w_settings.exec();
    switch (res)
    {
        case 0:
        {

            break;
        }
        case 1:
        {
            m_Port = w_settings.Port();
            m_ConnLiveTimeout=w_settings.ConnLiveTimeout();
            m_ThreadsPerUser=w_settings.Threads();
            m_ConnOnScreenTimeout=w_settings.ConnOnScreenTimeout();

            w_settings.getUsers(m_Users);

            break;
        }
        default: break;
    }
}

void Manager::ReadParameters()
{
    m_Users.clear();
    m_Users.push_back(User("","")); // Anonimous user
    m_Users.push_back(User("user","user"));
    m_Users.push_back(User("john","john"));

    m_Users[0].links({"mail.ru","ya.ru","youtube.com","yandex.net","yastatic.net","mycdn.me","mover.uz"});
    m_Users[1].links({"mail.ru","ya.ru","youtube.com","yandex.net","yastatic.net","mycdn.me","mover.uz"});
    m_Users[2].links({"mail.ru","ya.ru","youtube.com","yandex.net","yastatic.net","mycdn.me","mover.uz"});

    //m_Port=
    //m_ConnectionTimeout=
    //m_ThreadsPerUser=

    //m_Users["john"]->links
}

void Manager::WriteParameters()
{

}
