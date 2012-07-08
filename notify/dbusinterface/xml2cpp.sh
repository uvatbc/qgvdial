#!/bin/bash

# Client proxy
qdbusxml2cpp -v -c QGVNotifyProxyIface -p qgvn_proxy.h:qgvn_proxy.cpp iface.xml

# Server stub adapter
qdbusxml2cpp -v -c QGVNotifyIfaceAdapter -a qgvn_adapter.h:qgvn_adapter.cpp iface.xml

