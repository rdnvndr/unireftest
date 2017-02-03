#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlQueryModel>

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

private:
    void ExecFindQuery(const QString &strQuery);

    Ui::MainWindow *ui;
    QSqlQueryModel *m_model;
};

#endif // MAINWINDOW_H
