#!/bin/bash

pushd `dirname $0` > /dev/null

echo `pwd`
rm -f api_adapter.cpp api_adapter.h

qdbusxml2cpp -v -a api_adapter -v api_server.xml

popd >/dev/null
