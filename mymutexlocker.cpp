#include "mymutexlocker.h"

#include <QDebug>

QMap<QString, QMap<int, MyMutexLocker::lockInfo>> MyMutexLocker::allLocks;
QMutex MyMutexLocker::m_mutexallLocks;

MyMutexLocker::MyMutexLocker(QMutex* mutex, const QString& name, const QString& place)
    : m_Mutex(mutex)
    , m_Name(name)
    , m_Place(place)
    , m_Idx(0)
{
    //qDebug() << "LOCK " << m_Name << " IN " << m_Place;

    add();

    m_Mutex->lock();

    locked(true);
}


MyMutexLocker::~MyMutexLocker()
{
    //qDebug() << "UNLOCK " << m_Name;
    m_Mutex->unlock();    

    remove();
}

void MyMutexLocker::add()
{
    QMutexLocker lock(&m_mutexallLocks);

    auto it = MyMutexLocker::allLocks.find(m_Name);

    if (it!=MyMutexLocker::allLocks.end())
    {
        for(int i=0; ; i++)
        {
            auto it2 = MyMutexLocker::allLocks[m_Name].find(i);
            if (it2 == MyMutexLocker::allLocks[m_Name].end())
            {
                m_Idx=i;
                lockInfo info;
                info.place=m_Place;
                info.pThread = QThread::currentThread();
                info.mutex=m_Mutex;
                MyMutexLocker::allLocks[m_Name][i]=info;
                break;
            }
            else
            {
                if (MyMutexLocker::allLocks[m_Name][i].pThread==QThread::currentThread())
                {
                    qDebug() << m_Place << ", m_Name: Lock from same thread!!";
                    exit(1);
                }
            }
        }
    }
    else
    {
        lockInfo info;
        info.place=m_Place;
        info.pThread = QThread::currentThread();
        info.mutex=m_Mutex;
        MyMutexLocker::allLocks[m_Name].insert(m_Idx, info);
    }
}

void MyMutexLocker::remove()
{
    QMutexLocker lock(&m_mutexallLocks);

    auto it = MyMutexLocker::allLocks.find(m_Name);

    if (it!=MyMutexLocker::allLocks.end())
    {
        auto it2=MyMutexLocker::allLocks[m_Name].find(m_Idx);
        if (it2!=MyMutexLocker::allLocks[m_Name].end())
        {
            MyMutexLocker::allLocks[m_Name].erase(it2);
        }

        if (MyMutexLocker::allLocks[m_Name].size()==0)
        {
            MyMutexLocker::allLocks.erase(it);
        }
    }
}

void MyMutexLocker::locked(bool value)
{
    QMutexLocker lock(&m_mutexallLocks);
    MyMutexLocker::allLocks[m_Name][m_Idx].locked=value;
}
