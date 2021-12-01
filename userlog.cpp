#include "userlog.h"

#include <QMutexLocker>

UserLog::UserLog()
    : m_Size(0)
    , m_mutexSize()
{

}

void UserLog::addSize(unsigned long long value)
{
    QMutexLocker lock(&m_mutexSize);
    m_Size += value;
}
