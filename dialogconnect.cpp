#include "dialogconnect.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>

DialogConnect::DialogConnect(QWidget* pwgt) : QDialog(pwgt)
{
    setupUi(this);

    textLabelHostname->hide();
    editHostname->hide();
    textLabelBD->hide();
    editDatabase->hide();
    textLabelPort->hide();
    spinBoxPort->hide();
    line->hide();

    QSettings settings("NONAME", "TestUts");
    settings.beginGroup("DialogConnect");
    editDatabase->setText(settings.value("database").toString());
    editHostname->setText(settings.value("hostname").toString());
    editUsername->setText(settings.value("username").toString());
    spinBoxPort->setValue(settings.value("port", 1433).toInt());
    settings.endGroup();

    this->adjustSize();
    connect(pushButtonProp, SIGNAL(clicked()), this, SLOT(onClickButtonProp()));
    connect(pushButtonOk, SIGNAL(clicked()), this, SLOT(onClickButtonOk()));
}

void DialogConnect::onClickButtonProp()
{
    if (textLabelHostname->isHidden()) {
        pushButtonProp->setText(tr("Кратко"));
        textLabelHostname->show();
        editHostname->show();
        textLabelBD->show();
        editDatabase->show();
        textLabelPort->show();
        spinBoxPort->show();
        line->show();
    } else {
        pushButtonProp->setText(tr("Подробно"));
        textLabelHostname->hide();
        editHostname->hide();
        textLabelBD->hide();
        editDatabase->hide();
        textLabelPort->hide();
        spinBoxPort->hide();
        line->hide();

    }
    this->adjustSize();
}

void DialogConnect::onClickButtonOk()
{
    this->setDisabled(true);
    if (QSqlDatabase::database().isOpen()) {
        QSqlDatabase::database().close();
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }

//    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    QSqlDatabase db = QSqlDatabase::addDatabase("QTDS");
    db.setHostName(editHostname->text());
//    db.setDatabaseName(QString("%1;%2;%3").arg("DRIVER={SQL Server}")
//                       .arg("DATABASE=" + editDatabase->text())
//                       .arg("SERVER=" + editHostname->text()));
    db.setDatabaseName(editDatabase->text());
    db.setPort(spinBoxPort->value());
    db.setUserName(editUsername->text());
    db.setPassword(editPassword->text());

    if (!db.open()) {
        QSqlError err = db.lastError();
        if (err.type() != QSqlError::NoError){
            QMessageBox::warning(
                        NULL, tr("Не удается открыть базу данных"),
                        tr("Произошла ошибка при создании соединения: ")
                        + err.text());
            QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
        }
    } else {
            QSettings settings("NONAME", "TestUts");
            settings.beginGroup("DialogConnect");
            settings.setValue("database", editDatabase->text());
            settings.setValue("hostname", editHostname->text());
            settings.setValue("username", editUsername->text());
            settings.setValue("port", spinBoxPort->value());
            settings.endGroup();
            this->accept();
    }

    this->setEnabled(true);
}
