#ifndef DIALCANCELDLG_H
#define DIALCANCELDLG_H

#include <QtGui>

class DialCancelDlg : public QMessageBox
{
    Q_OBJECT

public:
    DialCancelDlg (const QString &strNum, QWidget *parent = 0);
    int doModal (const QString &strMyNumber);

private slots:
    void callStarted ();

private:
    QString         strContact;
};

#endif // DIALCANCELDLG_H
