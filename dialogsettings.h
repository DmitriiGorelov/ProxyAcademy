#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>

#include "types.h"
#include "user.h"

#define colUsersCount 5
#define colUsersName 0
#define colUsersPass 1
#define colUsersEnabled 2
#define colUsersLog 3
#define colUsersAddrList 4

namespace Ui {
class dialogSettings;
}

class dialogSettings : public QDialog
{
    Q_OBJECT

public:
    explicit dialogSettings(QWidget *parent = nullptr);
    ~dialogSettings();

    void Port(TPort value);
    TPort Port();

    void ConnLiveTimeout(int value);
    int ConnLiveTimeout();

    void ConnOnScreenTimeout(int value);
    int ConnOnScreenTimeout();

    void Threads(int value);
    int Threads();

    void setUsers(const tUsers& users);
    void getUsers(tUsers& users);
protected:
    void updateWindow();

private slots:
    void on_tableUsers_cellChanged(int row, int column);

    void userAddrListSelect();

    void on_tableUserTime_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_tableUsers_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

private:
    Ui::dialogSettings *ui;
    tUsers m_Users;

};

#endif // DIALOGSETTINGS_H
