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

    QueryManagerThread *queryThread = new QueryManagerThread(QSqlDatabase::database());
    queryThread->setText(ui->findLineEdit->text());
    connect(queryThread, &QueryManagerThread::resultReady,
            this, &MainWindow::onResult);
    connect(this, &MainWindow::stoped, queryThread, &QueryManagerThread::stop);

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

void MainWindow::onResult(QString value)
{
    m_list.append(value);
    m_model->setStringList(m_list);
    m_finish = QDateTime::currentDateTime();
    int msecs = m_finish.time().msecsTo(m_start.time());
    ui->logPlainText->appendPlainText(
                QString("\nЗапрос выполнен за %1 мсек").arg(abs(msecs)));
}


void MainWindow::closeEvent(QCloseEvent *)
{
//    if (QSqlDatabase::database().isOpen()) {
//        QSqlDatabase::database().close();
//        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
//    }
}
