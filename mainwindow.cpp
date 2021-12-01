#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QNetworkProxyFactory>
#include <QStringList>

#include "manager.h"
#include "mymutexlocker.h"
#include <QtConcurrent/QtConcurrent>

// https://wiki.qt.io/How_to_Use_QTableWidget

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_items()
    , m_mutexItems()    
{
    ui->setupUi(this);

    QNetworkProxyFactory::setUseSystemConfiguration(false); // to avoid self-loop of proxy if proxy is configured in the system to this clientPort
    //QThreadPool::globalInstance()->setMaxThreadCount(10);

    ui->toolBar->actions().at(0)->setEnabled(true);
    ui->toolBar->actions().at(1)->setDisabled(true);
    ui->toolBar->actions().at(2)->setEnabled(true);

    QStringList labels={"Connection","Command","Size In","Size Out","User"};
    ui->tableWidget->setColumnCount(infoCount);
    ui->tableWidget->setHorizontalHeaderLabels(labels);
    ui->tableWidget->setAlternatingRowColors(true);
}

MainWindow::~MainWindow()
{        
    delete ui;
}

void MainWindow::on_actionStart_triggered()
{
    connect(Manager::Instance(), &Manager::closed, this, &MainWindow::ManagerClosed, Qt::ConnectionType::QueuedConnection);
    connect(Manager::Instance(), &Manager::ConnectionUpdated, this, &MainWindow::ConnectionUpdated, Qt::ConnectionType::QueuedConnection);
    connect(Manager::Instance(), &Manager::endPointClosed, this, &MainWindow::ConnectionClosed, Qt::ConnectionType::QueuedConnection);
    connect(Manager::Instance(), &Manager::endPointRemoved, this, &MainWindow::ConnectionRemoved, Qt::ConnectionType::QueuedConnection);

    Manager::Instance()->start();
    ui->toolBar->actions().at(0)->setDisabled(true);
    ui->toolBar->actions().at(1)->setEnabled(true);
    ui->toolBar->actions().at(2)->setDisabled(true);
}

void MainWindow::on_actionStop_triggered()
{
    Manager::Instance()->quit();
    ui->toolBar->actions().at(1)->setDisabled(true);
}

void MainWindow::ManagerClosed()
{
    ui->toolBar->actions().at(0)->setEnabled(true);
    ui->toolBar->actions().at(2)->setEnabled(true);

    {
        MyMutexLocker lock(&m_mutexItems, "mutexItems", PLACE);
        m_items.clear();
        ui->tableWidget->clearSelection();
        ui->tableWidget->disconnect();
        ui->tableWidget->clearContents();
        ui->tableWidget->setRowCount(0);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Manager::Instance()->quit();
}

QTableWidgetItem * MainWindow::newRow()
{
    int row = ui->tableWidget->rowCount();

    ui->tableWidget->insertRow(row);

    QTableWidgetItem * first = nullptr;

    for(int i =0; i < infoCount; i++)
    {
        QTableWidgetItem * item = new QTableWidgetItem;
        if(i == 0)
            first = item;
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->tableWidget->setItem(row,i,item);
    }

    if(first)
        ui->tableWidget->setCurrentItem(first);

    return first;
}

void MainWindow::ConnectionUpdated(int idx, connectioninfo info)
{
    if (info.blocked()!=allowedOK)
        return;

    MyMutexLocker lock(&m_mutexItems, "mutexItems", PLACE);

    auto it = m_items.find(info.handle());
    if (it==m_items.end())
    {
        int row = 0;//ui->tableWidget->rowCount();

        ui->tableWidget->insertRow(row);

        for(int i =0; i < infoCount; i++)
        {
            QTableWidgetItem * item = new QTableWidgetItem;
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);            
            ui->tableWidget->setItem(row,i,item);

            m_items[info.handle()][i] = item;
            switch(i)
            {
                case infoIdxName: item->setText(info.name()); break;
                case infoIdxMethod: item->setText(info.methodName()); break;
                case infoIdxSizeIn: item->setText(QString::number(info.sizeIn())); break;
                case infoIdxSizeOut: item->setText(QString::number(info.sizeOut())); break;
                //case infoIdxUser: item->setText(m_Users[info.user()].name()); break;
                case infoIdxAllowed: if (!info.blocked()) item->setText(""); else item->setText("BLOCKED"); break;
            }            
        }
        m_items[info.handle()][0]->setBackgroundColor(QColor(0,255,0));
    }
    else
    {
        //for(int i =0; i < infoCount; i++)
        if (idx>-1 && idx < infoCount)
        {
            auto* item = it.value()[idx];
            switch(idx)
            {
                case infoIdxSizeIn: item->setText(QString::number(info.sizeIn())); break;
                case infoIdxSizeOut: item->setText(QString::number(info.sizeOut())); break;
            }
        }
    }
}

void MainWindow::ConnectionClosed(qintptr handle)
{   
    MyMutexLocker lock(&m_mutexItems, "mutexItems", PLACE);

    auto it = m_items.find(handle);
    if (it != m_items.end())
    {
        if (it.value()[0])
        {
            it.value()[0]->setBackgroundColor(QColor(255,255,255));
        }
    }
}

void MainWindow::ConnectionRemoved(qintptr handle)
{
    MyMutexLocker lock(&m_mutexItems, "mutexItems", PLACE);

    auto it = m_items.find(handle);
    if (it != m_items.end())
    {
        if (it.value()[0])
        {
            int k = it.value()[0]->row();
            ui->tableWidget->removeRow(k);
            m_items.erase(it);
        }
    }
}

void MainWindow::on_actionSettings_triggered()
{
    Manager::Instance()->ShowSettingsDialog();
}
