#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>
#include <QSemaphore>
#include <QDateTime>
#include <QMutex>
#include <QQueue>
#include <QCache>

#include "querymanagerthread.h"

namespace Ui {
class MainWindow;
}

class QueryManagerThread;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void closeEvent(QCloseEvent *);

public slots:
    void onActionExec();
    void onRunAction();
    void onActionConnect();
    void onShowPool();
    void onResult(QString value);
    void onFreeThread(QueryManagerThread *thread, bool interruption);

signals:
    void stoped();

private:
    Ui::MainWindow *ui;
    QStringListModel *m_model;
    QStringList m_list;

    QDateTime m_start;
    QDateTime m_finish;

    QQueue<QueryManagerThread *> m_queue;
    QQueue<QueryThread *> m_tqueue;

    QCache<QString, QStringList> m_cache;
};

#endif // MAINWINDOW_H
