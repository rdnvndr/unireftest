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
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_model;
}

void MainWindow::onActionExec()
{
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

    QString fields("");
    QString table("");
    QString sql("ALTER TABLE %1 ADD showme VARCHAR(256) NULL;\n"
                "EXEC('UPDATE %1 SET showme = %2')");
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
        QString nameTable = metaDataQuery.value("NAMETABLE").toString();
        QString nameField = metaDataQuery.value("NAMEFIELD").toString();

        if (nameField != "SCREEN_NAME" && !nameField.isEmpty()) {
            if (table != nameTable) {
                if (table != "") {
                    findSql = sql.arg(table).arg(fields);
                    ExecFindQuery(findSql);
                    QSqlQuery query;
                    if (query.prepare("SELECT CONSTRAINT_NAME "
                                       "FROM INFORMATION_SCHEMA.TABLE_CONSTRAINTS "
                                       "WHERE TABLE_NAME='"+ table
                                       +"' AND CONSTRAINT_TYPE='PRIMARY KEY'"))
                    {
                        query.exec();
                    }

                    QString pr_key = query.first() ? query.value(0).toString() : "";
                    qDebug() << pr_key;
                    if (query.prepare("CREATE FULLTEXT INDEX ON "
                                       + table + "(showme) "
                                       "KEY INDEX " + pr_key
                                       + " ON (ft) WITH (CHANGE_TRACKING AUTO)"))
                    {
                        query.exec();
                    }
                }
                fields = "CAST(" + nameField + " AS varchar(250))";
                //expr = nameField;
                table = nameTable;
            } else {
                fields += "+ ' ' + CAST(" + nameField + " AS varchar(250))";

            }
        }
    }

    if (table != "") {

        findSql = sql.arg(table).arg(fields);
        ExecFindQuery(findSql);
        QSqlQuery query;
        if (query.prepare("SELECT CONSTRAINT_NAME "
                          "FROM INFORMATION_SCHEMA.TABLE_CONSTRAINTS "
                          "WHERE TABLE_NAME='"+ table
                          +"' AND CONSTRAINT_TYPE='PRIMARY KEY'"))
        {
            query.exec();
        }
        QString pr_key = query.first() ? query.value(0).toString() : "";
        qDebug() << pr_key;
        if (query.prepare("CREATE FULLTEXT INDEX ON "
                          + table + "(showme) "
                          "KEY INDEX " + pr_key
                          + " ON (ft) WITH (CHANGE_TRACKING AUTO)"))
        {
            query.exec();
        }
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
    //SELECT CONSTRAINT_NAME FROM INFORMATION_SCHEMA.TABLE_CONSTRAINTS
    //WHERE TABLE_NAME='WORK_CM' AND CONSTRAINT_TYPE='PRIMARY KEY'
    //CREATE FULLTEXT INDEX ON WORK_CM(showme)
    //KEY INDEX PK_WORK_CM ON (ft) WITH (CHANGE_TRACKING AUTO)

    QSqlQuery query;
    if (!query.prepare(strQuery))
        return;
    if (!query.exec()) {
        return;
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
