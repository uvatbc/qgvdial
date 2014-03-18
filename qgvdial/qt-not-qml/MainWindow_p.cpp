/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2014  Yuvraaj Kelkar

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

#include "MainWindow_p.h"
#include "ui_mainwindow.h"

MainWindowPrivate::MainWindowPrivate(QWidget *parent)
: QMainWindow(parent)
, ui(new Ui::MainWindow)
, m_allowClose(true)
{
    ui->setupUi(this);
}//MainWindowPrivateLLMainWindowPrivate

MainWindowPrivate::~MainWindowPrivate()
{
    delete ui;
}//MainWindowPrivate::~MainWindowPrivate

void
MainWindowPrivate::setOrientation(ScreenOrientation orientation)
{
#if defined(Q_OS_SYMBIAN)
    // If the version of Qt on the device is < 4.7.2, that attribute won't work
    if (orientation != ScreenOrientationAuto) {
        const QStringList v = QString::fromAscii(qVersion()).split(QLatin1Char('.'));
        if (v.count() == 3 && (v.at(0).toInt() << 16 | v.at(1).toInt() << 8 | v.at(2).toInt()) < 0x040702) {
            Q_WARN("Screen orientation locking only supported with Qt 4.7.2 and above");
            return;
        }
    }
#endif // Q_OS_SYMBIAN

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    Qt::WidgetAttribute attribute;
    switch (orientation) {
#if QT_VERSION < QT_VERSION_CHECK(4,7,2)
    // Qt < 4.7.2 does not yet have the Qt::WA_*Orientation attributes
    case ScreenOrientationLockPortrait:
        attribute = static_cast<Qt::WidgetAttribute>(128);
        break;
    case ScreenOrientationLockLandscape:
        attribute = static_cast<Qt::WidgetAttribute>(129);
        break;
    default:
    case ScreenOrientationAuto:
        attribute = static_cast<Qt::WidgetAttribute>(130);
        break;
#else // QT_VERSION >= QT_VERSION_CHECK(4,7,2)
    case ScreenOrientationLockPortrait:
        attribute = Qt::WA_LockPortraitOrientation;
        break;
    case ScreenOrientationLockLandscape:
        attribute = Qt::WA_LockLandscapeOrientation;
        break;
    default:
    case ScreenOrientationAuto:
        attribute = Qt::WA_AutoOrientation;
        break;
#endif
    };
    setAttribute(attribute, true);
#else
    Q_UNUSED(orientation);
#endif
}//MainWindowPrivate::setOrientation

void
MainWindowPrivate::showExpanded()
{
#if defined(Q_OS_SYMBIAN) || defined(Q_WS_SIMULATOR)
    showFullScreen();
#elif defined(Q_WS_MAEMO_5)
    showMaximized();
#else
    show();
#endif
}//MainWindowPrivate::showExpanded

void
MainWindowPrivate::closeEvent(QCloseEvent *event)
{
    if (m_allowClose) {
        QMainWindow::closeEvent (event);
    } else {
        event->ignore ();
        this->hide ();
    }
}//MainWindowPrivate::closeEvent

void
MainWindowPrivate::setAllowClose(bool allow)
{
    m_allowClose = allow;
}//MainWindowPrivate::setAllowClose
