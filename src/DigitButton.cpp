#include "DigitButton.h"

DigitButton::DigitButton (QChar chr, QString strSubText,
                          QWidget *parent/* = 0*/):
QBaseButton(parent),
dispChar (chr)
{
    QObject::connect (this, SIGNAL (clicked ()),
                      this, SLOT   (_i_clicked ()));
    this->setText (chr);

#ifdef Q_WS_MAEMO_5
    this->setValueText (strSubText);
    this->setValueLayout (QMaemo5ValueButton::ValueUnderTextCentered);
#else
    Q_UNUSED (strSubText);
#endif
}//DigitButton::DigitButton

DigitButton::~DigitButton(void)
{
}//DigitButton::~DigitButton

void
DigitButton::_i_clicked ()
{
    emit clickedChar (dispChar);
}//DigitButton::_i_clicked
