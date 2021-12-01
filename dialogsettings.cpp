#include <QStyledItemDelegate>
#include <QPainter>
#include <QPushButton>
#include <QSharedPointer>
#include <QFileDialog>
#include <QFile>

#include "dialogsettings.h"
#include "ui_dialogsettings.h"
#include "manager.h"

Q_DECLARE_METATYPE( QTableWidgetItem* )

class DelegatePass : public QStyledItemDelegate
{
public:
    DelegatePass(QObject* parent = nullptr)
        : QStyledItemDelegate(parent)
    {

    }

public:

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        painter->drawText(option.rect, Qt::AlignCenter, "******");
    }
};

dialogSettings::dialogSettings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::dialogSettings)
    , m_Users()
{
    ui->setupUi(this);
    DelegatePass* delegate = new DelegatePass(ui->tableUsers);
    ui->tableUsers->setItemDelegateForColumn(colUsersPass,delegate);
}

dialogSettings::~dialogSettings()
{
    delete ui;
}

void dialogSettings::Port(TPort value)
{
    ui->spinPort->setValue(value);
}

TPort dialogSettings::Port()
{
    return ui->spinPort->value();
}

void dialogSettings::ConnLiveTimeout(int value)
{
    ui->spinConnectionLiveTimeout->setValue(value);
}

int dialogSettings::ConnLiveTimeout()
{
    return ui->spinConnectionLiveTimeout->value();
}

void dialogSettings::ConnOnScreenTimeout(int value)
{
    ui->spinConnectionOnScreenTimeout->setValue(value);
}

int dialogSettings::ConnOnScreenTimeout()
{
    return ui->spinConnectionOnScreenTimeout->value();
}

void dialogSettings::Threads(int value)
{
    ui->spinThreads->setValue(value);
}

int dialogSettings::Threads()
{
    return ui->spinThreads->value();
}

/*namespace Qt
{
  template<class T, class... Args>
  QSharedPointer<T> make_shared(Args&&... args)
  {
    return QSharedPointer<T>(new T(std::forward<Args>(args)...));
  }
}*/

void dialogSettings::setUsers(const tUsers& users)
{
    m_Users=users;
    updateWindow();
}

void dialogSettings::getUsers(tUsers& users)
{
    for (int r=0; r<ui->tableUsers->rowCount(); r++)
    {
        QString userName=ui->tableUsers->item(r,0)->text();
        //users[userName] = QSharedPointer<User>(new User(userName,""));

        for (int c=0; c<ui->tableUsers->columnCount(); c++)
        {
            switch(c)
            {
                case colUsersName:
                {
                    m_Users[r].name(ui->tableUsers->item(r,c)->text());
                    break;
                }
                case colUsersPass:
                {
                    m_Users[r].pass(ui->tableUsers->item(r,c)->data(Qt::UserRole).toString());
                    break;
                }
                case colUsersEnabled:
                {
                    m_Users[r].enabled(ui->tableUsers->item(r,c)->data(Qt::UserRole).toBool());
                    break;
                }
                case colUsersLog:
                {
                    m_Users[r].log(ui->tableUsers->item(r,c)->data(Qt::UserRole).toBool());
                    break;
                }
                case colUsersAddrList:
                {
                    //if (m_Users.find(userName)!=m_Users.end())
                    /*{
                        QString fileName = ui->tableUsers->item(r,c)->text();

                        m_Users[r].linksFile(fileName);
                    }*/

                    break;
                }
            }
        }
    }

    users=m_Users;
    //users[0]=m_Users[1]; // to demonstrate operator=() in User class
}

void dialogSettings::updateWindow()
{
    ui->tableUsers->setRowCount(0);
    ui->tableUsers->setColumnCount(colUsersCount);
    QStringList labels{"Name","Password", "Enabled", "Save log", "Addresses"};
    ui->tableUsers->setHorizontalHeaderLabels(labels);
    ui->tableUsers->setAlternatingRowColors(true);

    /*for (auto& it : m_Users)
    {
        int row = 0;
        ui->tableUsers->insertRow(row);
    }*/

    for (auto& it : m_Users)
    {
        int row = ui->tableUsers->rowCount();
        ui->tableUsers->insertRow(row);


        for(int i =0; i < colUsersCount; i++)
        {
            QTableWidgetItem * item = new QTableWidgetItem;
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->tableUsers->setItem(row,i,item);

            switch(i)
            {
                case colUsersName:
                {
                    item->setText(it.name());
                    break;
                }
                case colUsersPass:
                {
                    item->setData(Qt::DisplayRole, it.pass());
                    item->setData(Qt::UserRole, it.pass());
                    break;
                }
                case colUsersEnabled:
                {
                    item->setCheckState(it.enabled()?Qt::Checked:Qt::Unchecked);
                    break;
                }
                case colUsersLog:
                {
                    item->setCheckState(it.log()?Qt::Checked:Qt::Unchecked);
                    break;
                }
                case colUsersAddrList:
                {
                    QPushButton *browseButton = new QPushButton("...", ui->tableUsers);
                    browseButton->setProperty("linkfile",QVariant::fromValue(item));
                    connect(browseButton, &QPushButton::released, this, &dialogSettings::userAddrListSelect);
                    ui->tableUsers->setCellWidget(row, colUsersAddrList, browseButton);
                    item->setText(it.linksFile());
                    break;
                }
            }
        }
    }
}

void dialogSettings::on_tableUsers_cellChanged(int row, int column)
{
    switch (column)
    {
        case colUsersPass:
        {
            if (row>0) // avoid changing anonimous user
            {
                ui->tableUsers->item(row,column)->setData(Qt::UserRole, ui->tableUsers->item(row,column)->text());
            }
            break;
        }
        case colUsersEnabled:
        {
            ui->tableUsers->item(row,column)->setData(Qt::UserRole, ui->tableUsers->item(row,column)->checkState());
            break;
        }
        case colUsersLog:
        {
            ui->tableUsers->item(row,column)->setData(Qt::UserRole, ui->tableUsers->item(row,column)->checkState());
            break;
        }
    }
}

void dialogSettings::userAddrListSelect()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (btn)
    {
        //QSharedPointer<User> user = btn->property("user").value<QSharedPointer<User> >();
        QTableWidgetItem* item = btn->property("linkfile").value<QTableWidgetItem*>();

        // how to make field empty?
        if (item)
        {
            QFileDialog* editor = new QFileDialog(this);

            QString file(editor->getOpenFileName());

            if (file.isEmpty())
                return;

            item->setText(file);
        }
    }
}

void dialogSettings::on_tableUserTime_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    qDebug() << currentRow << ":" << currentColumn;
}

void dialogSettings::on_tableUsers_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    qDebug() << currentRow << ":" << currentColumn;

    //QString userName=ui->tableUsers->item(currentRow,0)->text();
    //m_Users[userName]=QSharedPointer<User>(new User(userName,""));
}
