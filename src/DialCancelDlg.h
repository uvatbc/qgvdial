#ifndef DIALCANCELDLG_H
#define DIALCANCELDLG_H

#include "global.h"

class DialCancelDlg : public QMessageBox
{
    Q_OBJECT

public:
    DialCancelDlg (const QString &strNum, QWidget *parent = 0);
    int doModal (const QString &strMyNumber);
    void doNonModal (const QString &strMyNumber);

signals:
    void dialDlgDone (int retval, const QString &strNumber);

public slots:
    void done (int r);

private slots:
    void callStarted ();

private:
    QString         strContact;
};

#endif // DIALCANCELDLG_H
