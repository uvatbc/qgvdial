#include "ChildWindowBase.h"
#include "Singletons.h"

ChildWindowBase::ChildWindowBase(QWidget *parent, Qt::WindowFlags  f)
: ChildWindowBaseClass(parent, f)
{
    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.setDefaultWindowAttributes (this);
}//ChildWindowBase::ChildWindowBase
