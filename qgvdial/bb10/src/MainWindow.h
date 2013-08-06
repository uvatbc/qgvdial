/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

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

// Tabbed pane project template
#ifndef MainWindow_HPP_
#define MainWindow_HPP_

#include "IMainWindow.h"

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>

#include <QLocale>
#include <QTranslator>

// include JS Debugger / CS Profiler enabler
// this feature is enabled by default in the debug build only
#include <Qt/qdeclarativedebug.h>

using namespace bb::cascades;

namespace bb { namespace cascades { class Application; }}

/*!
 * @brief Application pane object
 *
 *Use this object to create and init app UI, to create context objects, to register the new meta types etc.
 */
class MainWindow : public IMainWindow
{
    Q_OBJECT

public:
    MainWindow(QCoreApplication *app);
    virtual ~MainWindow() {}

    void uiRequestLoginDetails();
    void uiRequestTFALoginDetails(void*);
    void uiSetUserPass(const QString &user, const QString &pass,
                       bool editable);

    void init();

private:
    QmlDocument *qml;
    AbstractPane *root;
};

QCoreApplication *
createApplication(int argc, char **argv);

#endif /* MainWindow_HPP_ */
