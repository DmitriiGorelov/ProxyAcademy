#ifndef MYMUTEXLOCKER_H
#define MYMUTEXLOCKER_H

#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QMap>
#include <QAtomicInt>
#include <QThread>

class MyMutexLocker
{
public:
    MyMutexLocker(QMutex* mutex, const QString& name, const QString& place="");

    ~MyMutexLocker();

private:
    struct lockInfo
    {
        QString place="";
        QThread* pThread=nullptr;
        bool locked=false;
        QMutex* mutex=nullptr;
    };
    static QMap<QString, QMap<int, lockInfo> > allLocks;
    static QMutex m_mutexallLocks;

    void add();
    void remove();

    void locked(bool value);

private:

    QMutex* m_Mutex;
    QString m_Name;
    QString m_Place;
    QAtomicInt m_Idx;
};

#endif // MYMUTEXLOCKER_H
