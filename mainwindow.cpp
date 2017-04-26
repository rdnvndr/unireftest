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

    QString findString = "'%"
            + ui->findLineEdit->text().simplified().replace(' ', '%')
            + "%'";
    QString fields("");
    QString expr("");
    QString table("");
    QString sql("SELECT TOP " + QString("%1").arg(MAX_COUNT)
                + " GUID, %1 AS NAME FROM %2 WHERE %3\n");
    QString findSql("");

    QSqlQuery metaDataQuery(
                "SELECT BO_CLASSES.NAMECLASS AS NAMECLASS,\n"
                       "BO_CLASSES.NAMETABLE AS NAMETABLE,\n"
                       "BO_CLASSES.FGUID AS CLS_FGUID,\n"
                       "BO_ATTR_CLASSES.NAMEATTR AS NAMEATTR,\n"
                       "BO_ATTR_CLASSES.NAMEFIELD AS NAMEFIELD,\n"
                       "BO_ATTR_CLASSES.NAMESCREEN AS NAMESCREEN\n"
                "FROM BO_ATTR_CLASSES \n"
                "LEFT OUTER JOIN BO_CLASSES\n"
                "ON BO_CLASSES.GUID = BO_ATTR_CLASSES.FGUID\n"
                "INNER JOIN BO_ATTR_CLASSES AS t3\n"
                "ON t3.NAMEFIELD = 'GUID' AND t3.FGUID = BO_CLASSES.GUID\n"
                "AND not t3.NAMEFIELD is NULL\n"
                "WHERE substring(BO_ATTR_CLASSES.ARRAYMDATA, 3,1) = '1'\n"
                "ORDER BY CLS_FGUID, NAMETABLE");
    metaDataQuery.setForwardOnly(true);

    while (metaDataQuery.next()) {
        m_mutex.lock();
        if (m_count >= MAX_COUNT) {
            m_mutex.unlock();
            break;
        } else m_mutex.unlock();
        QString nameTable = metaDataQuery.value("NAMETABLE").toString();
        QString nameField = metaDataQuery.value("NAMEFIELD").toString();

        if (nameField != "SCREEN_NAME" && !nameField.isEmpty()) {
            if (table != nameTable) {
                if (table != "") {
                    findSql = sql.arg(fields).arg(table).arg(expr);
                    ExecFindQuery(findSql);
                }
                fields = "CAST(" + nameField + " AS varchar(250))";
                expr = nameField + " like " + findString;
                table = nameTable;
            } else {
                fields += "+ ' ' + CAST(" + nameField + " AS varchar(250))";
                expr += " or " + nameField + " like " + findString;
            }
        }
    }

    if (table != "") {
        m_mutex.lock();
        if (m_count < MAX_COUNT) {
            m_mutex.unlock();
            findSql = sql.arg(fields).arg(table).arg(expr);
            ExecFindQuery(findSql);
        } else m_mutex.unlock();
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
