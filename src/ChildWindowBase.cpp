#include "ChildWindowBase.h"

ChildWindowBase::ChildWindowBase(QWidget *parent, Qt::WindowFlags  f)
: ChildWindowBaseClass(parent, f)
{
#ifdef Q_WS_MAEMO_5
    this->setAttribute (Qt::WA_Maemo5StackedWindow);
    this->setAttribute (Qt::WA_Maemo5AutoOrientation);
#endif
}//ChildWindowBase::ChildWindowBase
