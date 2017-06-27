#ifndef QUERYMANAGERTHREAD_H
#define QUERYMANAGERTHREAD_H

#include <QThread>
#include <QObject>
#include <QSqlDatabase>
#include <QMutex>
#include <QSemaphore>
#include <QQueue>
#include <QSqlQuery>

#include "querythread.h"

class QueryManagerThread : public QThread
{
    Q_OBJECT
public:
    explicit QueryManagerThread(QSqlDatabase db, QObject * parent = 0);
    virtual ~QueryManagerThread();

    void run();

    void execQuery(const QString &strQuery);

    QString text() const;
    void setText(const QString &text);

    void setQueue(QQueue<QueryThread *> *queue);

public slots:
    void start();
    void stop();

    bool dbConnect();
    void finishQuery();
    void onResult(QString value);
    void onFreeThread(QueryThread *thread);

signals:
    void resultReady(QString value);
    void stoped();
    void freeThread(QueryManagerThread *thread);

private:
    QString m_driverName;
    QString m_databaseName;
    QString m_hostName;
    int     m_port;
    QString m_userName;
    QString m_password;
    QString m_text;

    QMutex m_mutex;
    int    m_count;
    bool   m_stop;

    QSemaphore m_threadCount;
    QQueue<QueryThread *> *m_queue;
    QString connName;
    QSqlQuery query;
};

#endif // QUERYMANAGERTHREAD_H
