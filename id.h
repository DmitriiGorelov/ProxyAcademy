#ifndef ID_H
#define ID_H

#include <QString>

class uid
{
    static unsigned long long _id;
public:
    uid()
    {
        m_id =  _id++;
    }

    unsigned long long id()
    {
        return m_id;
    }

    QString ids()
    {
        return QString::number(m_id);
    }

private:
    unsigned long long m_id;
};

#endif // ID_H
