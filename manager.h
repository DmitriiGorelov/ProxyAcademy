#ifndef MANAGER_H
#define MANAGER_H

#include <QObject>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThreadPool>
#include <QThread>
#include <QSharedPointer>
#include <QMap>
#include <QList>
#include <QSharedPointer>
#include <QFuture>
#include <QMap>

#include "types.h"
#include "client.h"
#include "endpoint.h"
#include "user.h"
#include "dialogsettings.h"

#define infoCount 7
#define infoIdxName 0
//#define infoIdxAddress 1
//#define infoIdxPort 2
#define infoIdxMethod 1
#define infoIdxSizeIn 2
#define infoIdxSizeOut 3
#define infoIdxUser 4
#define infoIdxAllowed 5

class Manager : public QTcpServer
{
    Q_OBJECT
public:
    static Manager* Instance()
    {
        if (!Manager::manager)
        {
            Manager::manager=new Manager(nullptr);
        }
        return Manager::manager;
    }

    void addConnection(qintptr handle, QSharedPointer<Endpoint> endpoint, QSharedPointer<Client> client);
    void removeConnection(qintptr handle);
    void stopConnections();

    TPort Port();
    int ConnTimeout();
    int ThreadsPerUser();

    void ShowSettingsDialog();

private:
    explicit Manager(QObject *parent = nullptr);

signals:
    void closed();
    void ConnectionUpdated(int idx, connectioninfo info);
    void endPointClosed(qintptr handle);
    void endPointRemoved(qintptr handle);

public slots:
    void start();
    void quit();

public:
    bool isRunning();

    void infoSizeIn(qintptr handle, unsigned long long size);
    void infoSizeOut(qintptr handle, unsigned long long size);

    void ReadParameters();
    void WriteParameters();

    // QTcpServer interface
protected:
    //Not version friendly!!!
    virtual void incomingConnection(qintptr handle) Q_DECL_OVERRIDE;

    static void newBridge(qintptr handle);
    void callConnectiontInit(connectioninfo* info);
private:
    static Manager* manager;

    class connection {
    public:
        connection()
        {

        }

        connection(connectioninfo _info,QSharedPointer<Endpoint> _endpoint, QSharedPointer<Client> _client)
            : endpoint(_endpoint)
            , client(_client)
            , info(_info)
        {

        }

        QSharedPointer<Endpoint> endpoint;
        QSharedPointer<Client> client;
        connectioninfo info;
    };

    QMap<qintptr,  connection> connections;
    QMutex m_connectionsMutex;

    QAtomicInt m_Running;

    tUsers m_Users; // key - login

    dialogSettings w_settings;
    TPort m_Port;
    int m_ConnLiveTimeout;
    int m_ConnOnScreenTimeout;
    int m_ThreadsPerUser;
};

#endif // MANAGER_H
