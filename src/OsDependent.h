#ifndef __OSDEPENDENT_H__
#define __OSDEPENDENT_H__

#include "global.h"

#if TELEPATHY_CAPABLE
#include <TelepathyQt4/Constants>
#include <TelepathyQt4/Types>
#if !NO_DBGINFO
#include <TelepathyQt4/Debug>
#endif
#endif

#ifdef Q_WS_MAEMO_5
#include <QtMaemo5>
#endif

class OsDependent : public QObject
{
    Q_OBJECT

public:
    void init ();
    bool isN900 ();
    void initDialServer (QObject *receiver, const char *method);
    void setDefaultWindowAttributes (QWidget *pWidget);

private:
    OsDependent(QObject *parent = 0);

    friend class Singletons;
};

#ifdef QT_NO_SYSTEMTRAYICON
class QSystemTrayIcon : public QWidget
{
    Q_OBJECT
public:
    QSystemTrayIcon(QWidget *parent = 0);
    QSystemTrayIcon(const QIcon &icon, QWidget *parent = 0);
    ~QSystemTrayIcon();

    static bool isSystemTrayAvailable();
    static bool supportsMessages();

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    QString toolTip() const;
    void setToolTip(const QString &tip);

    enum ActivationReason {
        Unknown,
        Context,
        DoubleClick,
        Trigger,
        MiddleClick
    };

Q_SIGNALS:
    void activated(QSystemTrayIcon::ActivationReason reason);
};
#endif

#endif //__OSDEPENDENT_H__
