#ifndef USER_H
#define USER_H

#include <QString>
#include <QMutex>
#include <QSharedPointer>
#include <QFile>
#include <QDebug>
#include <QVector>

#include "types.h"
#include "connectioninfo.h"
#include "userlog.h"

class User
{
public:    
    User();
    User(QString _name, QString _pass);
    User(const User& h);
    User& operator =(const User& h);


    ~User()
    {
        qDebug() << " Destroy user " << name();
    }

    QString pass() const {return m_Pass;}
    void pass(QString value) { m_Pass=value;}

    void name(QString value) {m_Name = value;}
    QString name() const {return m_Name;}

    bool enabled() const { return m_bEnabled; }
    void enabled(bool value) { m_bEnabled=value;}

    bool log() const { return m_Log;}
    void log(bool value) {m_Log=value;}

    void newConnection(connectioninfo* info);

    bool addSize(QString name, tDataSize value);

    void maxSize(tDataSize value) {m_MaxSize=value;}
    tDataSize maxSize() const { return m_MaxSize; }

    void totalSize(tDataSize value){m_TotalSize=value;}
    tDataSize totalSize() const { return m_TotalSize; }

    bool open();
    void close();

    void linksFile(const QString& value);
    QString linksFile() const;

    void links(QStringList value);
    QStringList links() const;

    tLogData logData() const {return m_LogData; };

protected:
    void logWrite(const QString& text);

private:
    QString m_Name;
    QString m_Pass;
    bool m_bEnabled;
    bool m_Log;
    QString m_LinksFile;
    QStringList m_Links;

    QFile m_LogFile;

    tLogData m_LogData;
    QMutex m_mutexLog;

    tDataSize m_TotalSize;
    tDataSize m_MaxSize;


};

typedef QVector< User > tUsers;

#endif // USER_H
