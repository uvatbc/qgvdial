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

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class OsDependent : public QObject
{
    Q_OBJECT

public:
    void init ();
    bool isN900 ();
    void initDialServer (QObject *receiver, const char *method);
    void initTextServer (QObject *r1, const char *m1,
                         QObject *r2, const char *m2);
    void setDefaultWindowAttributes (QWidget *pWidget);
    void setLongWork (QWidget *window, bool bSet = false);

private:
    OsDependent(QObject *parent = 0);

    friend class Singletons;
};

#ifdef QT_NO_SYSTEMTRAYICON
class QSystemTrayIcon : public QWidget
{
    Q_OBJECT
public:
    enum ActivationReason {
        Unknown,
        Context,
        DoubleClick,
        Trigger,
        MiddleClick
    };

    enum MessageIcon {
        Information
    };

    QSystemTrayIcon(QWidget *parent = 0);
    QSystemTrayIcon(const QIcon &icon, QWidget *parent = 0);
    ~QSystemTrayIcon();

    static bool isSystemTrayAvailable();
    static bool supportsMessages();

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    QString toolTip() const;
    void setToolTip(const QString &tip);

    void setContextMenu (QMenu *menu);

    void showMessage (const QString &title,
                      const QString &message,
                      MessageIcon icon = Information,
                      int millisecondsTimeoutHint = 10000);

Q_SIGNALS:
    void activated(QSystemTrayIcon::ActivationReason reason);
};
#endif

#endif //__OSDEPENDENT_H__
