#include <QtCore>
#include "LoginDialog.h"
#include "ui_LoginDialog.h"

LoginDialog::LoginDialog (QString &strU, QString &strP, QWidget *parent)
: QDialog(parent)
, ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    ui->edUsername->setText (strU);
    ui->edPassword->setText (strP);
}//LoginDialog::LoginDialog

LoginDialog::~LoginDialog ()
{
    delete ui;
}//LoginDialog::~LoginDialog

bool
LoginDialog::getUserPass (QString &strUser, QString &strPass)
{
    if (QDialog::Accepted != this->result ()) {
        return (false);
    }

    strUser = ui->edUsername->text ();
    strPass = ui->edPassword->text ();

    return (true);
}//LoginDialog::getUserPass
