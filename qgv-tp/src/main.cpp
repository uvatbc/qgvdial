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

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusInterface>
#include <QDateTime>

#include "names.h"
#include "connectionmanager.h"
#include "basetypes.h"
#include "connectionmanagertypes.h"
#include "connectiontypes.h"
#include "connectioninterfacerequeststypes.h"
#include "connectioninterfacecapabilitiestypes.h"


using namespace std;

ofstream logfile;

void MyOutputHandler(QtMsgType type, const char *msg) {
    switch (type) {
        case QtDebugMsg:
            logfile << QTime::currentTime().toString().toAscii().data() << " Debug: " << msg << "\n";
            break;
        case QtCriticalMsg:
            logfile << QTime::currentTime().toString().toAscii().data() << " Critical: " << msg << "\n";
            break;
        case QtWarningMsg:
            logfile << QTime::currentTime().toString().toAscii().data() << " Warning: " << msg << "\n";
            break;
        case QtFatalMsg:
            logfile << QTime::currentTime().toString().toAscii().data() <<  " Fatal: " << msg << "\n";
            abort();
    }
}

int main(int argc, char ** argv)
{

//    logfile.open("/var/log/logfile.txt", ios::app);
//    #ifndef QT_NO_DEBUG_OUTPUT
//    qInstallMsgHandler(MyOutputHandler);
//    #endif


    QCoreApplication app(argc, argv);

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

    if (!connection.interface()->isServiceRegistered(cm_service_name))
    {

        // register CM on D-BUS:
        if (connection.registerService(cm_service_name)){
            qDebug(qPrintable(QObject::tr("Service %1 registered with session bus.").
                       arg(cm_service_name)));
        }
        else{
            qDebug(qPrintable(QObject::tr("Unable to register service %1 with session bus.").
                       arg(cm_service_name)));
        }

    }

    ConnectionManager connection_mgr(&app);
    if (!connection.registerObject(cm_object_path,&connection_mgr)){
        qDebug(qPrintable(QObject::tr("Unable to register VICaR connection manager at path %1 with session bus.").
                   arg(cm_object_path)));
    }

    qDebug(qPrintable(QObject::tr("Entering main loop.")));    
//    logfile.close();
    return app.exec();
}
