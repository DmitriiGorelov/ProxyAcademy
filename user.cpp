#include <QDateTime>
#include <QTextStream>
#include <QDebug>

#include "user.h"

User::User()
    : m_Name("")
    , m_Pass("")
    , m_bEnabled(true)
    , m_Log(false)
    , m_LinksFile("")
    , m_Links()
    , m_LogFile()
    , m_LogData()
    , m_TotalSize(0)
    , m_MaxSize(0)
{

}

User::User(QString _name, QString _pass)
    : m_Name(_name)
    , m_Pass(_pass)
    , m_bEnabled(true)
    , m_Log(false)
    , m_LinksFile("")
    , m_Links()
    , m_LogFile()
    , m_LogData()
    , m_TotalSize(0)
    , m_MaxSize(0)
{

}

User::User(const User& h)
    : m_Name(h.name())
    , m_Pass(h.pass())
    , m_bEnabled(h.enabled())
    , m_Log(h.log())
    , m_LinksFile(h.linksFile())
    , m_Links(h.links())
    , m_LogFile()
    , m_LogData(h.logData())
    , m_TotalSize(h.totalSize())
    , m_MaxSize(h.maxSize())
{

}

User& User::operator =(const User& h)
{
    return *this;
}

void User::newConnection(connectioninfo* info)
{
    if (!enabled())
    {
        info->blocked(blockedUserDisabled);
    }

    if (!addSize(info->name(), info->sizeIn())) // no matter, size here is 0. We only check if we have size capacity to open new connection for this user.
    {
        info->blocked(blockedSize);
    }

    for (auto it : m_Links)
    {
        if (info->address().contains(it,Qt::CaseSensitivity::CaseInsensitive))
        {
            info->blocked(blockedLink);
        }
    }

    logWrite(info->address()+"\t"+QString::number(info->blocked()));
}

bool User::addSize(QString name, tDataSize value)
{
    QMutexLocker lock(&m_mutexLog);
    //m_Log[name].addSize(value);
    m_LogData[name] += value;

    m_TotalSize+=value;

    return (m_TotalSize <= m_MaxSize) || (m_MaxSize == 0);
}

bool User::open()
{
    if (!m_LogFile.isOpen())
    {
        QDateTime t = QDateTime::currentDateTime();
        m_LogFile.setFileName("d:/ProxyLog_"+m_Name+"_"+t.toString("dd_MM_yyyy")+".log");
        return m_LogFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
    }
    return true;
}

void User::close()
{
    if (m_LogFile.isOpen())
        m_LogFile.close();
}

void User::linksFile(const QString &value)
{
    /*m_LinksFile = value;
    //m_Links.clear();

    if (m_LinksFile.isEmpty())
        return;

    QFile file(m_LinksFile);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);

    while(!in.atEnd())
        m_Links.append(in.readLine());

    file.close();

    qDebug() << m_LinksFile << m_Links;*/
}

QString User::linksFile() const
{
    return m_LinksFile;
}

void User::links(QStringList value)
{
    m_Links=value;
}

QStringList User::links() const
{
    return m_Links;
}

void User::logWrite(const QString& text)
{
    if (!m_Log)
        return;

    if (m_LogFile.isOpen())
    {
        QTextStream out(&m_LogFile);

        QDateTime t = QDateTime::currentDateTime();

        out << t.toString("dd.MM.yyyy hh:mm:ss") << "\t" << text << endl;
    }
}
