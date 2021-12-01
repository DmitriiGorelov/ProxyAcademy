#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QVector>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QCloseEvent>

#include "connectioninfo.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QTableWidgetItem * newRow();

private slots:
    void on_actionStart_triggered();
    void on_actionStop_triggered();

    void ConnectionUpdated(int idx, connectioninfo info);
    void ConnectionRemoved(qintptr handle);
    void ConnectionClosed(qintptr handle);
    void ManagerClosed();
    void on_actionSettings_triggered();

private:
    void closeEvent (QCloseEvent *event);
private:
    Ui::MainWindow *ui;

    QMap<qintptr,  QMap<int, QTableWidgetItem* > > m_items;
    QMutex m_mutexItems;    
};

#endif // MAINWINDOW_H
