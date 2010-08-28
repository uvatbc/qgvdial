#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
    class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog (QString &strU, QString &strP, QWidget *parent = 0);
    ~LoginDialog();

    bool getUserPass (QString &strUser, QString &strPass);

private:
    Ui::LoginDialog *ui;
};

#endif // LOGINDIALOG_H
