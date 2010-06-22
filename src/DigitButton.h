#ifndef __DIGITBUTTON_H__
#define __DIGITBUTTON_H__

#include "global.h"

#include <QtCore>
#include <QtGui>

#ifdef Q_WS_MAEMO_5
#include <QtMaemo5>
typedef QMaemo5ValueButton QBaseButton;
#else
typedef QPushButton QBaseButton;
#endif // Q_WS_MAEMO_5

class DigitButton : public QBaseButton
{
    Q_OBJECT

public:
    DigitButton (QChar chr, QString strSubText, QWidget *parent = 0);
    virtual ~DigitButton(void);

private slots:
    void _i_clicked ();

signals:
    void clickedChar (QChar chr);

private:
    QChar dispChar;
};

#endif //__DIGITBUTTON_H__
