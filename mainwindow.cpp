#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogconnect.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>
#include <QSqlQuery>
#include <QDateTime>

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
    m_count = 0;
    m_list.clear();

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
    QDateTime start = QDateTime::currentDateTime();

    QString findString = "'%" + ui->findLineEdit->text() + "%'";
    QString fields("");
    QString expr("");
    QString table("");
    QString sql("SELECT TOP "
                + QString(MAX_COUNT) + " GUID, %1 AS NAME FROM %2 WHERE %3\n");
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

    while (metaDataQuery.next() && m_count < MAX_COUNT) {
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

    if (table != "" && m_count < MAX_COUNT) {
        findSql = sql.arg(fields).arg(table).arg(expr);
        ExecFindQuery(findSql);
    }

    QDateTime finish = QDateTime::currentDateTime();
    int msecs = finish.time().msecsTo(start.time());

    ui->logPlainText->appendPlainText(
                QString("\nЗапрос выполнен за %1 мсек").arg(abs(msecs)));
    m_model->setStringList(m_list);
}

void MainWindow::onActionConnect()
{
    DialogConnect* windowConnect = new DialogConnect(this);
    windowConnect->exec();
    delete windowConnect;
    return;
}

void MainWindow::ExecFindQuery(const QString &strQuery)
{
    QSqlQuery query(strQuery);
    while (query.next() && m_count < MAX_COUNT) {
        m_count++;
        m_list.append(query.value("NAME").toString());
    }
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
