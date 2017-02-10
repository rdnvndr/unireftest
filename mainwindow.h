#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>

namespace Ui {
class MainWindow;
}

const int MAX_COUNT = 10;

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

private:
    void ExecFindQuery(const QString &strQuery);

    Ui::MainWindow *ui;
    QStringListModel *m_model;
    QStringList m_list;
    int m_count;
};

#endif // MAINWINDOW_H
