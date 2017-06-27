#include "querymanagerthread.h"
#include <QVariant>

QueryManagerThread::QueryManagerThread(QSqlDatabase db, QObject *parent): QThread(parent)
{
    m_driverName = db.driverName();
    m_databaseName = db.databaseName();
    m_hostName = db.hostName();
    m_port = db.port();
    m_userName = db.userName();
    m_password = db.password();
    connName = "";
}

QueryManagerThread::~QueryManagerThread()
{
    QSqlDatabase::removeDatabase(connName);
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
    m_count = 0;
    QThread::start();
}

void QueryManagerThread::stop()
{
    m_stop = true;
    emit stoped();
}

bool QueryManagerThread::dbConnect()
{
    if (connName.isEmpty()) {
        QThread* curThread = QThread::currentThread();
        connName = QString("RTP0%1").arg(
                    reinterpret_cast<qlonglong>(curThread), 0, 16);
        QSqlDatabase db = QSqlDatabase::addDatabase(m_driverName, connName);
        db.setDatabaseName(m_databaseName);
        db.setHostName(m_hostName);
        db.setPort(m_port);
        db.setUserName(m_userName);
        db.setPassword(m_password);
        if (!db.open()) {
            return false;
        }
        query = QSqlQuery(db);
        query.prepare(
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
        query.setForwardOnly(true);
    }
    return true;
}

void QueryManagerThread::finishQuery()
{
    m_threadCount.acquire();
    if (m_threadCount.available() == 0)
        emit freeThread(this);
}

void QueryManagerThread::onResult(QString value)
{
    if (!m_stop)
        emit resultReady(value);
}

void QueryManagerThread::onFreeThread(QueryThread *thread)
{
    m_queue->enqueue(thread);
    finishQuery();
}

void QueryManagerThread::setQueue(QQueue<QueryThread *> *queue)
{
    m_queue = queue;
}

void QueryManagerThread::execQuery(const QString &strQuery)
{
    QueryThread *queryThread;
    if (!m_queue->isEmpty()) {
        queryThread = m_queue->dequeue();
        disconnect(queryThread, &QueryThread::resultReady, 0, 0);
        disconnect(queryThread, &QueryThread::freeThread, 0, 0);
    } else {
        queryThread = new QueryThread(QSqlDatabase::database());
        connect(this, &QObject::destroyed, queryThread, &QObject::deleteLater);
    }
    queryThread->setMutex(&m_mutex);
    queryThread->setCount(&m_count);
    connect(queryThread, &QueryThread::resultReady,
            this,  &QueryManagerThread::onResult);

    connect(queryThread, &QueryThread::freeThread,
            this, &QueryManagerThread::onFreeThread);

    connect(this,  &QueryManagerThread::stoped,
            queryThread, &QueryThread::stop);
    queryThread->setQueryText(strQuery);

    m_threadCount.release();
    queryThread->start();
}

void QueryManagerThread::run()
{
    m_stop = false;
    if (!dbConnect())
        return;

    query.exec();

    QString sql("SELECT TOP " + QString("%1").arg(MAX_COUNT)
                + " GUID, %1 AS NAME FROM %2 WHERE %3\n");

    QString findString = "'%"
            + m_text.simplified().replace(' ', '%')
            + "%'";
    QString findSql("");
    QString fields("");
    QString expr("");
    QString table("");

    while (query.next()) {
        m_mutex.lock();
        if (m_count < MAX_COUNT && !m_stop) {
            m_mutex.unlock();
        } else {
            m_mutex.unlock();
            break;
        }
        QString nameTable = query.value("NAMETABLE").toString();
        QString nameField = query.value("NAMEFIELD").toString();

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

    finishQuery();
}
