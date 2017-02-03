#ifndef DIALOGCONNECT_H
#define	DIALOGCONNECT_H
#include "ui_dialogconnect.h"
#include <QtGui>

class DialogConnect: public QDialog, public Ui::DialogConnect
{
    Q_OBJECT
public:
    DialogConnect(QWidget* pwgt = 0);
public slots:
    void onClickButtonProp();    
    void onClickButtonOk();
};

#endif
