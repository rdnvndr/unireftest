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
    m_connName = "";
}

QueryThread::~QueryThread()
{
    QSqlDatabase::removeDatabase(m_connName);
}

void QueryThread::run()
{
    m_stop = false;
    if (!dbConnect())
        return;

    if (!m_query.prepare(m_queryText))
        return;

    if (!m_query.exec())
        return;

    while (m_query.next()) {
        if (checkStop())
            break;
        emit resultReady(m_query.value("NAME").toString());
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
    if (m_connName.isEmpty()) {
        QThread* curThread = QThread::currentThread();
        m_connName = QString("RTP0%1").arg(
                    reinterpret_cast<qlonglong>(curThread), 0, 16);

        QSqlDatabase db = QSqlDatabase::addDatabase(m_driverName, m_connName);
        db.setDatabaseName(m_databaseName);
        db.setHostName(m_hostName);
        db.setPort(m_port);
        db.setUserName(m_userName);
        db.setPassword(m_password);
        if (!db.open())
            return false;
        m_query = QSqlQuery(db);
        m_query.setForwardOnly(true);
    }
    return true;
}

bool QueryThread::checkStop()
{
    QMutexLocker locker(m_mutex);
    if (*m_count < MAX_COUNT && !m_stop) {
        (*m_count)++;
        return false;
    }
    return true;
}

void QueryThread::stop()
{
    m_stop = true;
}
