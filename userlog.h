#ifndef USERLOG_H
#define USERLOG_H

#include <QString>
#include <QMap>
#include <QMutex>

class UserLog
{
public:
    UserLog();

    void addSize(unsigned long long value);

private:
    unsigned long long m_Size;
    QMutex m_mutexSize;

};

#endif // USERLOG_H
