/*
@version: 0.5
@author: Sudheer K. <scifi1947 at gmail.com>
@license: GNU General Public License

Based on Telepathy-SNOM with copyright notice below.
*/

/*
 * Telepathy SNOM VoIP phone connection manager
 * Copyright (C) 2006 by basyskom GmbH
 *  @author Tobias Hunger <info@basyskom.de>
 *
 * This library is free software; you can redisQObject::tribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This library is disQObject::tributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin SQObject::treet, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <iostream>
#include <fstream>

#include <QtCore>
#include <QtDBus>

#include "names.h"
#include "connectionmanager.h"
#include "basetypes.h"
#include "connectionmanagertypes.h"
#include "connectiontypes.h"
#include "connectioninterfacerequeststypes.h"
#include "connectioninterfacecapabilitiestypes.h"


using namespace std;

ofstream logfile;

void
open_logfile ()
{
    if (!logfile.is_open ()) {
        logfile.open ("/home/user/.qgvdial/qgv-tp.log", ios::app);
    }
}

void dbgHandler(QtMsgType type, const char *msg)
{
    QDateTime dt = QDateTime::currentDateTime ();
    int level = 0;

    switch (type) {
        case QtDebugMsg:
            level = 3;
            break;
        case QtWarningMsg:
            level = 2;
            break;
        case QtCriticalMsg:
            level = 1;
            break;
        case QtFatalMsg:
            level = 0;
            break;
    }

    QString strLog = QString("%1 : %2 : %3")
                     .arg(dt.toString ("yyyy-MM-dd hh:mm:ss.zzz"))
                     .arg(level)
                     .arg(msg);
    open_logfile ();
    logfile << strLog.toAscii().data() << endl;
    cout << strLog.toAscii().data() << endl;

    if (QtFatalMsg == type) {
        abort ();
    }
}

int
main(int argc, char ** argv)
{
    QCoreApplication app(argc, argv);
    QString msg;

    open_logfile ();
    qInstallMsgHandler(dbgHandler);

    // register types:
    qDBusRegisterMetaType<org::freedesktop::Telepathy::ParameterDefinition>();
    qDBusRegisterMetaType<org::freedesktop::Telepathy::ParameterDefinitionList>();
    qDBusRegisterMetaType<org::freedesktop::Telepathy::ChannelInfo>();
    qDBusRegisterMetaType<org::freedesktop::Telepathy::ChannelInfoList>();
    qDBusRegisterMetaType<org::freedesktop::Telepathy::ChannelDetails>();
    qDBusRegisterMetaType<org::freedesktop::Telepathy::ChannelDetailsList>();
    qDBusRegisterMetaType<org::freedesktop::Telepathy::ContactCapabilities>();
    qDBusRegisterMetaType<org::freedesktop::Telepathy::ContactCapabilitiesList>();
    qDBusRegisterMetaType<org::freedesktop::Telepathy::CapabilityPair>();
    qDBusRegisterMetaType<org::freedesktop::Telepathy::CapabilityPairList>();
    qDBusRegisterMetaType<org::freedesktop::Telepathy::CapabilityChange>();
    qDBusRegisterMetaType<org::freedesktop::Telepathy::CapabilityChangeList>();
    qDBusRegisterMetaType<org::freedesktop::Telepathy::RequestableChannelClass>();
    qDBusRegisterMetaType<org::freedesktop::Telepathy::RequestableChannelClassList>();

    QDBusConnection connection = QDBusConnection::sessionBus();

    if (!connection.interface()->isServiceRegistered(cm_service_name)) {
        // register CM on D-BUS:
        if (connection.registerService(cm_service_name)) {
            msg = QString ("Service %1 registered with session bus.")
                    .arg(cm_service_name);
            qDebug() << msg;
        } else {
            msg = QString ("Unable to register service %1 with session bus.")
                    .arg(cm_service_name);
            qDebug() << msg;
        }
    }

    ConnectionManager connection_mgr(&app);
    if (!connection.registerObject(cm_object_path,&connection_mgr)) {
        msg = QString ("Unable to register VICaR connection manager at path %1 with session bus.")
                .arg(cm_object_path);
        qDebug() << msg;
    }

    qDebug("Entering main loop.");
    int rv = app.exec();
    logfile.close();
    return rv;
}//main
