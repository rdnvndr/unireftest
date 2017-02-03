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
    m_model = new QSqlQueryModel(this);
    ui->resultTableView->setModel(m_model);
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
    QString sql("SELECT GUID, %1 AS NAME FROM %2 WHERE %3\n");
    QString findSql("");

    QSqlQuery metaDataQuery(
                "SELECT BO_CLASSES.NAMECLASS AS NAMECLASS, \
                        BO_CLASSES.NAMETABLE AS NAMETABLE, \
                        BO_CLASSES.FGUID AS CLS_FGUID, \
                        BO_ATTR_CLASSES.NAMEATTR AS NAMEATTR, \
                        BO_ATTR_CLASSES.NAMEFIELD AS NAMEFIELD, \
                        BO_ATTR_CLASSES.NAMESCREEN AS NAMESCREEN\
                FROM BO_ATTR_CLASSES \
                LEFT OUTER JOIN BO_CLASSES \
                ON BO_CLASSES.GUID = BO_ATTR_CLASSES.FGUID \
                INNER JOIN BO_ATTR_CLASSES AS t3 \
                ON t3.NAMEFIELD = 'GUID' AND t3.FGUID = BO_CLASSES.GUID \
                   AND not t3.NAMEFIELD is NULL \
                WHERE substring(BO_ATTR_CLASSES.ARRAYMDATA, 3,1) = '1' \
                ORDER BY CLS_FGUID, NAMETABLE");


    while (metaDataQuery.next()) {
        QString nameTable = metaDataQuery.value("NAMETABLE").toString();
        QString nameField = metaDataQuery.value("NAMEFIELD").toString();

        if (nameField != "SCREEN_NAME" && !nameField.isEmpty()) {
            if (table != nameTable) {
                if (table != "") {
                    findSql += sql.arg(fields).arg(table).arg(expr);
                    sql = "UNION ALL SELECT GUID, %1 AS NAME FROM %2 WHERE %3\n";
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
        findSql += sql.arg(fields).arg(table).arg(expr);
    }
    findSql = "SELECT TOP 10 * FROM ("+ findSql +") AS t";
    ExecFindQuery(findSql);
    QDateTime finish = QDateTime::currentDateTime();
    int msecs = finish.time().msecsTo(start.time());

//    ui->logPlainText->appendPlainText(findSql);
    ui->logPlainText->appendPlainText(
                QString("\nЗапрос выполнен за %1 мсек").arg(abs(msecs)));
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
    m_model->setQuery(strQuery);
}

void MainWindow::closeEvent(QCloseEvent *)
{
//    if (QSqlDatabase::database().isOpen()) {
//        QSqlDatabase::database().close();
//        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
//    }
}
