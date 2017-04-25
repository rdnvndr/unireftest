#ifndef QUERYTHREAD_H
#define QUERYTHREAD_H

#include <QThread>
#include <QObject>
#include <QSqlDatabase>
#include <QSemaphore>

class QueryThread : public QThread
{
    Q_OBJECT
public:
    explicit QueryThread(QSqlDatabase db, QObject * parent = 0);
    void run();

    QString queryText() const;
    void setQueryText(const QString &queryText);

signals:
    void resultReady(QStringList values);

private:
    QString m_driverName;
    QString m_databaseName;
    QString m_hostName;
    int     m_port;
    QString m_userName;
    QString m_password;
    QString m_queryText;
};

#endif // QUERYTHREAD_H
