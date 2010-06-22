#include "SMSEntryDeleteButton.h"

SMSEntryDeleteButton::SMSEntryDeleteButton(int ind, QWidget *parent/* = 0*/) :
QPushButton (parent),
index(ind)
{
    this->setText ("Delete");
    QObject::connect (this, SIGNAL (clicked()),
                      this, SLOT   (btnClicked ()));
}//SMSEntryDeleteButton::SMSEntryDeleteButton

SMSEntryDeleteButton::~SMSEntryDeleteButton (void)
{
}//SMSEntryDeleteButton::~SMSEntryDeleteButton

void
SMSEntryDeleteButton::btnClicked ()
{
    emit triggered (index);
}//SMSEntryDeleteButton::btnClicked

void
SMSEntryDeleteButton::setIndex (int i)
{
    index = i;
}//SMSEntryDeleteButton::setIndex
