#include "querythread.h"

#include <QSqlQuery>
#include <QStringList>
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
}

void QueryThread::run()
{
    QThread* curThread = QThread::currentThread();
    QString connName = QString("RTP0%1").arg((qlonglong)curThread, 0, 16);
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
        bool ret = query->prepare(m_queryText);
        ret = query->exec();
        QStringList list;
        while (query->next()) {
            list.append(query->value("NAME").toString());
        }
        emit resultReady(list);
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

