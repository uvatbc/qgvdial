INCLUDEPATH += $$PWD

QT *= dbus

HEADERS  += $$PWD/gen/api_adapter.h  \
            $$PWD/DBusApi.h
SOURCES  += $$PWD/gen/api_adapter.cpp \
            $$PWD/DBusApi.cpp

DEFINES += DBUS_API

maemo5 | contains(MEEGO_EDITION,harmattan) {
dbus_services.source = $$PWD/service_files/optified/qgvdial.Call.service \
                       $$PWD/service_files/optified/qgvdial.Text.service \
                       $$PWD/service_files/optified/qgvdial.UI.service
} else {
dbus_services.source = $$PWD/service_files/regular/qgvdial.Call.service \
                       $$PWD/service_files/regular/qgvdial.Text.service \
                       $$PWD/service_files/regular/qgvdial.UI.service
}

# Since we're using the QT auto-deployment feature (that always attempts to optify EVERYTHING),
# this path munging is required whether it is Maemo, Harmattan or Ubuntu
dbus_services.target = ../../usr/share/dbus-1/services

# Stupid
contains($$PWD,export) {
message(Export!)
DEPLOYMENTFOLDERS += dbus_services
}
