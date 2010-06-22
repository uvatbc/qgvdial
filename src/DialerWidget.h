#ifndef __DIALERWIDGET_H__
#define __DIALERWIDGET_H__

#include "global.h"
#include <QtGui>

class DialerWidget : public QWidget
{
    Q_OBJECT

public:
    DialerWidget (QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~DialerWidget ();

signals:
    //! Log emitter
    void log(const QString &strText, int level = 10);
    //! Status emitter for status bar
    void status(const QString &strText, int timeout = 2000);

    //! Emitted when user requests a call
    void call (const QString &strTarget, const QString &strSource);
    //! Emitted when user requests a call
    void call (const QString &strTarget);

public slots:
    void updateMenu (QMenuBar *menuBar);

private slots:
    void digitClicked (QChar chr);
    void backspace ();

    void btnCallClicked ();

private:
    QGridLayout     grid;
    QLineEdit       edNumber;
};

#endif //__DIALERWIDGET_H__
