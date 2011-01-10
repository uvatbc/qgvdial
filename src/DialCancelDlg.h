#ifndef DIALCANCELDLG_H
#define DIALCANCELDLG_H

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

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
