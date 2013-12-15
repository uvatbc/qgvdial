#!/bin/bash
make -j4
blackberry-nativepackager -package qgvdial.bar bar-descriptor.xml -devMode -debugToken /media/drobo/uv/work/bb_dev_cert/z10-dbgtoken.bar

