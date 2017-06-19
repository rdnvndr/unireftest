#ifndef QUERYMANAGERTHREAD_H
#define QUERYMANAGERTHREAD_H

#include <QThread>
#include <QObject>
#include <QSqlDatabase>
#include <QMutex>
#include <QSemaphore>

class QueryManagerThread : public QThread
{
    Q_OBJECT
public:
    explicit QueryManagerThread(QSqlDatabase db, QObject * parent = 0);
    void run();

    void execQuery(const QString &strQuery);

    QString text() const;
    void setText(const QString &text);

public slots:
    void start();
    void stop();

    void finishQuery();
    void onResult(QString value);

signals:
    void resultReady(QString value);
    void stoped();

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
};

#endif // QUERYMANAGERTHREAD_H
