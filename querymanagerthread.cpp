#include "querymanagerthread.h"

#include <QSqlQuery>
#include <QVariant>
#include "querythread.h"

QueryManagerThread::QueryManagerThread(QSqlDatabase db, QObject *parent): QThread(parent)
{
    m_driverName = db.driverName();
    m_databaseName = db.databaseName();
    m_hostName = db.hostName();
    m_port = db.port();
    m_userName = db.userName();
    m_password = db.password();
    m_count = 0;

    connect(this, &QueryManagerThread::finished, this, &QueryManagerThread::finishQuery);
}

QString QueryManagerThread::text() const
{
    return m_text;
}

void QueryManagerThread::setText(const QString &text)
{
    m_text = text;
}

void QueryManagerThread::start()
{
    m_threadCount.release();
    QThread::start();
}

void QueryManagerThread::stop()
{
    m_stop = true;
    emit stoped();
}

void QueryManagerThread::finishQuery()
{
    m_threadCount.acquire();
    if (m_threadCount.available() == 0)
        this->deleteLater();
}

void QueryManagerThread::onResult(QString value)
{
    if (!m_stop)
        emit resultReady(value);
}

void QueryManagerThread::execQuery(const QString &strQuery)
{
    QueryThread *queryThread = new QueryThread(QSqlDatabase::database());
    queryThread->setQueryText(strQuery);
    queryThread->setMutex(&m_mutex);
    queryThread->setCount(&m_count);

    connect(queryThread, &QueryThread::resultReady,
            this,  &QueryManagerThread::onResult);
    connect(queryThread, &QueryThread::finished,
            queryThread, &QObject::deleteLater);

    connect(this,  &QueryManagerThread::stoped,
            queryThread, &QueryThread::stop);

    m_threadCount.release();
    queryThread->start();
}

void QueryManagerThread::run()
{
    m_stop = false;
    QThread* curThread = QThread::currentThread();
    QString connName = QString("RTP0%1").arg(
                reinterpret_cast<qlonglong>(curThread), 0, 16);
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(m_driverName, connName);
        db.setDatabaseName(m_databaseName);
        db.setHostName(m_hostName);
        db.setPort(m_port);
        db.setUserName(m_userName);
        db.setPassword(m_password);

        if (!db.open()) {
            return;
        }

        QString findString = "'%"
                + m_text.simplified().replace(' ', '%')
                + "%'";
        QString fields("");
        QString expr("");
        QString table("");
        QString sql("SELECT TOP " + QString("%1").arg(MAX_COUNT)
                    + " GUID, %1 AS NAME FROM %2 WHERE %3\n");
        QString findSql("");

        QSqlQuery *query = new QSqlQuery(
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
                    "ORDER BY CLS_FGUID, NAMETABLE", db);
        query->setForwardOnly(true);


        while (query->next()) {
            m_mutex.lock();
            if (m_count < MAX_COUNT && !m_stop) {
                m_mutex.unlock();
            } else {
                m_mutex.unlock();
                break;
            }
            QString nameTable = query->value("NAMETABLE").toString();
            QString nameField = query->value("NAMEFIELD").toString();

            if (nameField != "SCREEN_NAME" && !nameField.isEmpty()) {
                if (table != nameTable) {
                    if (table != "") {
                        findSql = sql.arg(fields).arg(table).arg(expr);
                        execQuery(findSql);
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
            if (m_count < MAX_COUNT && !m_stop) {
                m_mutex.unlock();
                findSql = sql.arg(fields).arg(table).arg(expr);
                execQuery(findSql);
            } else m_mutex.unlock();
        }
        query->finish();
        delete query;
        db.close();
    }
    QSqlDatabase::removeDatabase(connName);
}
