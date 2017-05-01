#ifndef QUERYTHREAD_H
#define QUERYTHREAD_H

#include <QThread>
#include <QObject>
#include <QSqlDatabase>
#include <QMutex>

const int MAX_COUNT = 10;

class QueryThread : public QThread
{
    Q_OBJECT
public:
    explicit QueryThread(QSqlDatabase db, QObject * parent = 0);
    void run();

    QString queryText() const;
    void setQueryText(const QString &queryText);

    void setCount(int *count);
    void setMutex(QMutex *value);

signals:
    void resultReady(QString value);

private:
    QString m_driverName;
    QString m_databaseName;
    QString m_hostName;
    int     m_port;
    QString m_userName;
    QString m_password;
    QString m_queryText;

    QMutex *m_mutex;
    int *m_count;
};

#endif // QUERYTHREAD_H
