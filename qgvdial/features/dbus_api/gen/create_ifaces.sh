#!/bin/bash

pushd `dirname $0` > /dev/null

echo `pwd`
rm -f api_adapter.cpp api_adapter.h

qdbusxml2cpp -a api_adapter api_server.xml

popd >/dev/null
