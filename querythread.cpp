#include "querythread.h"

#include <QVariant>

QueryThread::QueryThread(QSqlDatabase db, QObject *parent): QThread(parent)
{
    m_driverName = db.driverName();
    m_databaseName = db.databaseName();
    m_hostName = db.hostName();
    m_port = db.port();
    m_userName = db.userName();
    m_password = db.password();
    m_queryText = "";
    connName = "";
}

QueryThread::~QueryThread()
{
    QSqlDatabase::removeDatabase(connName);
}

void QueryThread::run()
{
    m_stop = false;
    if (!dbConnect())
        return;

    if (!query.prepare(m_queryText))
        return;

    if (!query.exec())
        return;

    while (query.next()) {
        m_mutex->lock();
        if (*m_count < MAX_COUNT && !m_stop) {
            (*m_count)++;
            m_mutex->unlock();
            emit resultReady(query.value("NAME").toString());
        } else {
            m_mutex->unlock();
            break;
        }

    }
    emit freeThread(this);
}

QString QueryThread::queryText() const
{
    return m_queryText;
}

void QueryThread::setQueryText(const QString &queryText)
{
    m_queryText = queryText;
}

void QueryThread::setCount(int *count)
{
    m_count = count;
}

void QueryThread::setMutex(QMutex *value)
{
    m_mutex = value;
}

bool QueryThread::dbConnect()
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
        if (!db.open())
            return false;
        query = QSqlQuery(db);
        query.setForwardOnly(true);
    }
    return true;
}

void QueryThread::stop()
{
    m_stop = true;
}
