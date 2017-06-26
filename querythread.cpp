#include "querythread.h"

#include <QSqlQuery>
#include <QStringList>
#include <QVariant>
#include <QDebug>

QueryThread::QueryThread(QSqlDatabase db, QObject *parent): QThread(parent)
{
    m_driverName = db.driverName();
    m_databaseName = db.databaseName();
    m_hostName = db.hostName();
    m_port = db.port();
    m_userName = db.userName();
    m_password = db.password();
    m_queryText = "";
}

void QueryThread::run()
{
    QThread* curThread = QThread::currentThread();
    m_stop = false;
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

        QSqlQuery *query = new QSqlQuery(db);

        // Текущая сессия
        query->exec("SELECT @@SPID");
        if (query->next()) {
            m_session = query->value(0).toInt();
            qDebug() << QString("KILL %1").arg(m_session);
        }

        if (!query->prepare(m_queryText))
            return;
        query->setForwardOnly(true);
        if (!query->exec())
            return;

        while (query->next()) {
            m_mutex->lock();
            if (*m_count < MAX_COUNT && !m_stop) {
                (*m_count)++;
                m_mutex->unlock();
                emit resultReady(query->value("NAME").toString());
            } else {
                m_mutex->unlock();
                break;
            }

        }
        query->finish();
        delete query;
        db.close();
    }
    QSqlDatabase::removeDatabase(connName);
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

void QueryThread::stop()
{
    QSqlQuery query;
    if (this->isRunning())
    {
        query.exec(QString("KILL %1").arg(m_session));
    }
    m_stop = true;
}
