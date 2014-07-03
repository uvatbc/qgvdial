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

#include "MainWindow.h"
#include "CQmlViewer.h"

#include <QGLWidget>
#include <QGLFormat>

#define MAIN_QML_PATH "app/native/qml/bb10/main.qml"

QApplication *
createAppObject(int &argc, char **argv)
{
#define DRAG_DIST 16
    int startDragDistance = QApplication::startDragDistance ();
    if (DRAG_DIST != startDragDistance) {
        Q_DEBUG(QString("Original startDragDistance = %1")
                .arg (startDragDistance));
        QApplication::setStartDragDistance (DRAG_DIST);
    }

    return createNormalAppObject (argc, argv);
}//createAppObject

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)

QQuickView *
createQQuickView()
{
    return new QQuickView();
}//createQQuickView

#endif

MainWindow::MainWindow(QObject *parent)
: QmlMainWindow(parent)
{
}//MainWindow::MainWindow

void
MainWindow::init()
{
    qreal val;
    int width = QApplication::desktop()->screenGeometry().size().width();
    if (width == 720) {
        Q_DEBUG("Its a Q1!");
        val = 0.7;
        m_view->engine()->rootContext()
                    ->setContextProperty("g_keypadScaleFactor1", val);
        val = 0.8;
        m_view->engine()->rootContext()
                    ->setContextProperty("g_keypadScaleFactor2", val);
    } else {
        val = 1.0;
        m_view->engine()->rootContext()
                    ->setContextProperty("g_keypadScaleFactor1", val);
        m_view->engine()->rootContext()
                    ->setContextProperty("g_keypadScaleFactor2", val);
    }

    QmlMainWindow::init ();

    // GL viewport increases performance on blackberry
    QGLFormat format = QGLFormat::defaultFormat();
    format.setSampleBuffers(false);
    QGLWidget *glWidget = new QGLWidget(format);
    glWidget->setAutoFillBackground(false);

    m_view->setViewport(glWidget);

    // More gfx performance
    m_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_view->setAttribute(Qt::WA_OpaquePaintEvent);
    m_view->setAttribute(Qt::WA_NoSystemBackground);
    m_view->viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    m_view->viewport()->setAttribute(Qt::WA_NoSystemBackground);

    // None of these seem to have any effect
    //m_view->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    //m_view->setOrientation(QmlApplicationViewer::ScreenOrientationLockPortrait);
    //m_view->setOrientation(QmlApplicationViewer::ScreenOrientationLockLandscape);

}//MainWindow::init

void
MainWindow::uiOpenBrowser(const QUrl &url)
{
    QDesktopServices::openUrl(url);
}//MainWindow::uiOpenBrowser

void
MainWindow::uiCloseBrowser()
{
}//MainWindow::uiCloseBrowser

QString
MainWindow::getMainQmlPath()
{
    return MAIN_QML_PATH;
}//MainWindow::getMainQmlPath
