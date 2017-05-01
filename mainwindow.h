#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>
#include <QSemaphore>
#include <QDateTime>
#include <QMutex>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void closeEvent(QCloseEvent *);

private slots:
    void onActionExec();
    void onActionConnect();
    void onExit(QString value);

private:
    void ExecFindQuery(const QString &strQuery);

    Ui::MainWindow *ui;
    QStringListModel *m_model;
    QStringList m_list;
    QDateTime m_start;
    QDateTime m_finish;
};

#endif // MAINWINDOW_H
