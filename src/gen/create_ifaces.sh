#!/bin/bash

rm -f api_adapter.cpp api_adapter.h

qdbusxml2cpp -v -a api_adapter -v api_server.xml

