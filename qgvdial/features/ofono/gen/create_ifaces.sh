#!/bin/bash

pushd `dirname $0` > /dev/null

echo `pwd`
rm -f ofono_voicecall.cpp ofono_voicecall.h

qdbusxml2cpp -v -a ofono_voicecall -v voicecall.xml

popd >/dev/null
