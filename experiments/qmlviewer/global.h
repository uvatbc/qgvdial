#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <QtCore>
//#include <QtGui>
#include <QtDeclarative>

#define Q_DEBUG(_s) qDebug() << QString("%1(%2): %3").arg(__FUNCTION__).arg(__LINE__).arg(_s)
#define Q_WARN(_s) qWarning() << QString("%1(%2): %3").arg(__FUNCTION__).arg(__LINE__).arg(_s)
#define Q_CRIT(_s) qCritical() << QString("%1(%2): %3").arg(__FUNCTION__).arg(__LINE__).arg(_s)

#endif //__GLOBAL_H__
