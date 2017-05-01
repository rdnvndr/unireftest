#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogconnect.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>
#include <QSqlQuery>
#include "querythread.h"

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
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_model;
}

void MainWindow::onActionExec()
{
    m_mutex.lock();
    m_count = 0;
    m_mutex.unlock();

    m_list.clear();
    m_model->setStringList(m_list);

    ui->logPlainText->clear();
    if (!QSqlDatabase::database().isOpen()) {
        ui->logPlainText->appendPlainText(
                    "Отсутсвует соединение с базой данных\n");
        return;
    }
    if (ui->findLineEdit->text().isEmpty()) {
        ui->logPlainText->appendPlainText("Введите строку поиска\n");
        return;
    }

    ui->logPlainText->appendPlainText("Запуск запроса\n");
    m_start = QDateTime::currentDateTime();

    QString findString = ui->findLineEdit->text().simplified().replace(' ', '*');
    QString sql("SELECT TOP " + QString("%1").arg(MAX_COUNT)
                + " GUID, SHOWME AS NAME FROM %1 WHERE CONTAINS(SHOWME,'\"*"
                + findString +"*\"')\n");
    QString findSql("");

    QSqlQuery metaDataQuery("SELECT BO_CLASSES.NAMETABLE AS NAMETABLE "
                            "FROM BO_CLASSES");
    metaDataQuery.setForwardOnly(true);

    while (metaDataQuery.next()) {
        m_mutex.lock();
        if (m_count >= MAX_COUNT) {
            m_mutex.unlock();
            break;
        } else m_mutex.unlock();
        QString nameTable = metaDataQuery.value("NAMETABLE").toString();
        findSql = sql.arg(nameTable);
        ExecFindQuery(findSql);
    }
}

void MainWindow::onActionConnect()
{
    DialogConnect* windowConnect = new DialogConnect(this);
    windowConnect->exec();
    delete windowConnect;
    return;
}

void MainWindow::onExit(QString value)
{
        m_list.append(value);
        m_model->setStringList(m_list);
        m_finish = QDateTime::currentDateTime();
        int msecs = m_finish.time().msecsTo(m_start.time());
        ui->logPlainText->appendPlainText(
                    QString("\nЗапрос выполнен за %1 мсек").arg(abs(msecs)));
}

void MainWindow::ExecFindQuery(const QString &strQuery)
{
    QueryThread *queryThread = new QueryThread(QSqlDatabase::database());
    queryThread->setQueryText(strQuery);
    queryThread->setMutex(&m_mutex);
    queryThread->setCount(&m_count);

    connect(queryThread, &QueryThread::resultReady, this, &MainWindow::onExit);
    connect(queryThread, &QueryThread::finished, queryThread, &QObject::deleteLater);
    queryThread->start();
    if (ui->showQueryAction->isChecked())
        ui->logPlainText->appendPlainText(strQuery);
}

void MainWindow::closeEvent(QCloseEvent *)
{
//    if (QSqlDatabase::database().isOpen()) {
//        QSqlDatabase::database().close();
//        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
//    }
}
