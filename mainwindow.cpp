#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogconnect.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>
#include <QSqlQuery>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->connectAction, &QAction::triggered, this, &MainWindow::onActionConnect);
    connect(ui->runAction, &QAction::triggered, this, &MainWindow::onActionExec);
    connect(ui->exitAction, &QAction::triggered, this, &MainWindow::close);
    m_model = new QStringListModel(this);
    ui->resultTableView->setModel(m_model);
    connect(ui->findLineEdit, &QLineEdit::textChanged, this, &MainWindow::onActionExec);
    connect(ui->poolAction, &QAction::triggered, this, &MainWindow::onShowPool);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_model;
}

void MainWindow::onActionExec()
{

    ui->logPlainText->clear();
    if (!QSqlDatabase::database().isOpen()) {
        ui->logPlainText->appendPlainText(
                    "Отсутствует соединение с базой данных\n");
        return;
    }

    if (ui->findLineEdit->text().isEmpty()) {
        ui->logPlainText->appendPlainText("Введите строку поиска\n");
        return;
    }

    emit stoped();
    m_list.clear();
    m_model->setStringList(m_list);

    QueryManagerThread *queryThread;
    if (!m_queue.isEmpty())
    {
        queryThread = m_queue.dequeue();
        disconnect(queryThread, &QueryManagerThread::resultReady, 0, 0);
        disconnect(queryThread, &QueryManagerThread::freeThread, 0, 0);
    } else {
        queryThread = new QueryManagerThread(QSqlDatabase::database());
        queryThread->setQueue(&m_tqueue);
        connect(this, &MainWindow::stoped, queryThread, &QueryManagerThread::stop);
        connect(this, &QObject::destroyed, queryThread, &QObject::deleteLater);
    }
    connect(queryThread, &QueryManagerThread::resultReady,
            this, &MainWindow::onResult);
    connect(queryThread, &QueryManagerThread::freeThread,
            this, &MainWindow::onFreeThread);
    queryThread->setText(ui->findLineEdit->text());


    ui->logPlainText->appendPlainText("Запуск запроса\n");
    m_start = QDateTime::currentDateTime();
    queryThread->start();
}

void MainWindow::onActionConnect()
{
    DialogConnect* windowConnect = new DialogConnect(this);
    windowConnect->exec();
    delete windowConnect;
    return;
}

void MainWindow::onShowPool()
{
     ui->logPlainText->appendPlainText(
                 QString("\nПул менеджеров: %1\nПул запросов: %2\n")
                 .arg(m_queue.count())
                 .arg(m_tqueue.count()));
}

void MainWindow::onResult(QString value)
{
    if (ui->sortAction->isChecked()) {
        unsigned int left = 0;
        unsigned int right = m_list.count();
        unsigned int mid = 0;
        QString listValue;

        while ( left < right) {
            mid = trunc((left + right - 1) / 2);
            listValue = m_list.value(mid);

            if (value == listValue)
                break;

            if (QString::compare(value, listValue, Qt::CaseInsensitive) < 0) {
                right = mid;
            } else {
                ++mid;
                left = mid;
            }
        }
        m_list.insert(mid, value);
    } else {
        m_list.append(value);
    }
    m_model->setStringList(m_list);
    m_finish = QDateTime::currentDateTime();
    int msecs = m_finish.time().msecsTo(m_start.time());
    ui->logPlainText->appendPlainText(
                QString("\nЗапрос выполнен за %1 мсек").arg(abs(msecs)));
}

void MainWindow::onFreeThread(QueryManagerThread *thread)
{
    m_queue.enqueue(thread);
}

void MainWindow::closeEvent(QCloseEvent *)
{
    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }
}
