#!/bin/bash
make -j4
blackberry-nativepackager -package qgvdial.bar bar-descriptor.xml -devMode -debugToken ~/.rim/debugtoken1.bar

