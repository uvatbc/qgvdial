INCLUDEPATH += $$PWD

QT *= dbus

HEADERS  += $$PWD/gen/api_adapter.h  \
            $$PWD/DBusApi.h
SOURCES  += $$PWD/gen/api_adapter.cpp \
            $$PWD/DBusApi.cpp

dbus_services.source = $$PWD/service_files/qgvdial.Call.service \
                       $$PWD/service_files/qgvdial.Text.service \
                       $$PWD/service_files/qgvdial.UI.service
maemo5 {
dbus_services.target = ../../usr/share/dbus-1/services
}
DEPLOYMENTFOLDERS += dbus_services

