/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

#ifndef __OSDEPENDENT_H__
#define __OSDEPENDENT_H__

#include "global.h"

#ifdef Q_WS_MAEMO_5
#include <QtMaemo5>
#endif

#include <openssl/aes.h>
#include <openssl/evp.h>

#if defined(Q_OS_SYMBIAN) || defined(Q_WS_MAEMO_5)
#define SYSTEMDISPLAYINFO 0
#else
#include <QtSystemInfo/QSystemDisplayInfo>
#define SYSTEMDISPLAYINFO 1
QTM_USE_NAMESPACE
#endif

#if HAS_FEEDBACK
#include <QFeedbackHapticsEffect>
QTM_USE_NAMESPACE
#endif

#if TELEPATHY_CAPABLE
#include "DBusApi.h"
#endif

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

enum OsIndependentOrientation {
    OIO_Unknown = 0,
    OIO_Landscape,
    OIO_Portrait,
    OIO_InvertedLandscape,
    OIO_InvertedPortrait
};

class OsDependent : public QObject
{
    Q_OBJECT

public:
    void init ();

    bool isN900 ();

    bool isSymbian();
    bool isSymbian1();
    bool isSymbian3();

    bool ensureDBusObject();

    void initApiServer();

    void setDefaultWindowAttributes (QWidget *pWidget);
    void setLongWork (QWidget *window, bool bSet = false);

    QRect getStartingSize ();
    QString getAppDirectory ();
    QString getMainQML();
    void getMultipliers(double &hMul, double &wMul, double &fontMul);

    bool cipher(const QByteArray &byIn, QByteArray &byOut, bool bEncrypt);

    QString getOSDetails();

    OsIndependentOrientation getOrientation(void);

    void onBtnClickFroHapticFeedback();

signals:
    void orientationChanged (OsIndependentOrientation o);

////////////////////////////////////////////////////////////////////////////////
    // Signals from the dial, text and settings servers -> mainwindow
    void dialNow (const QString &strNumber);
    void sendText (const QStringList &arrNumbers,
                   const QString     &strData);
    void sendTextWithoutData (const QStringList &arrNumbers);
    bool phoneIndexChange(int index);

    // Signals from mainwindow -> dial, text and settings servers
    void phoneChanges(const QStringList &phones, int index);
////////////////////////////////////////////////////////////////////////////////

private:
    OsDependent(QObject *parent = 0);

private slots:
#if SYSTEMDISPLAYINFO
    //! Invoked when the desktop is resized (useful only on mobile platforms)
    void onOrientationChanged(QSystemDisplayInfo::DisplayOrientation o);
#else
    void desktopResized(int screen);
#endif

private:
#if SYSTEMDISPLAYINFO
    QSystemDisplayInfo *displayInfo;
#endif

#if HAS_FEEDBACK
    QFeedbackHapticsEffect *btnClickBuzz;
#endif

    bool bDBusObjectRegistered;
#if TELEPATHY_CAPABLE
    QGVDBusCallApi     *dbusCallApi;
    QGVDBusTextApi     *dbusTextApi;
    QGVDBusSettingsApi *dbusSettingsApi;
#endif

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
