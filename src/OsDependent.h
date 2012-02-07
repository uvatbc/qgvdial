/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

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

#if TELEPATHY_CAPABLE
#include <TelepathyQt4/Constants>
#include <TelepathyQt4/Types>
#ifdef DBG_TP_VERBOSE
#include <TelepathyQt4/Debug>
#endif
#endif

#ifdef Q_WS_MAEMO_5
#include <QtMaemo5>
#endif

#include <openssl/aes.h>
#include <openssl/evp.h>

#if !defined(Q_OS_SYMBIAN)
#include <QtSystemInfo/QSystemDisplayInfo>
QTM_USE_NAMESPACE
#endif

#ifndef QTM_VERSION
#define QTM_VERSION 0x010000
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
    void initDialServer (QObject *receiver, const char *method);
    void initTextServer (QObject *r1, const char *m1,
                         QObject *r2, const char *m2);
    void initSettingsServer(QObject *r1, const char *m1,
                            QObject *r2, const char *m2);
    void setDefaultWindowAttributes (QWidget *pWidget);
    void setLongWork (QWidget *window, bool bSet = false);

    QRect getStartingSize ();
    QString getAppDirectory ();
    QString getMainQML();

    bool cipher(const QByteArray &byIn, QByteArray &byOut, bool bEncrypt);

    QString getOSDetails();

    OsIndependentOrientation getOrientation(void);

signals:
    void orientationChanged (OsIndependentOrientation o);

private:
    OsDependent(QObject *parent = 0);

private slots:
#if QTM_VERSION >= 0x010200
    //! Invoked when the desktop is resized (useful only on mobile platforms)
    void onOrientationChanged(QSystemDisplayInfo::DisplayOrientation o);
#else
    void desktopResized(int screen);
#endif

private:
#if QTM_VERSION >= 0x010200
    QSystemDisplayInfo  displayInfo;
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
