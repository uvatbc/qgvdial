#!/bin/bash

rm -f cm_* connection_* protocol_* gen/cm_* gen/connection_* gen/protocol_*

#qdbusxml2cpp -v -a cm_adapter -p cm_proxy -v -i shared_data_types.h cm.xml
#qdbusxml2cpp -v -a connection_adapter -p connection_proxy -v -i shared_data_types.h connection.xml
#qdbusxml2cpp -v -a protocol_adapter -p protocol_proxy -v -i shared_data_types.h protocol.xml

qdbusxml2cpp -v -a cm_adapter -v -i shared_data_types.h -l QGVConnectionManager cm.xml
echo cm.xml done.
qdbusxml2cpp -v -a connection_adapter -v -i shared_data_types.h -l QGVConnection connection.xml
echo connection.xml done.
qdbusxml2cpp -v -a protocol_adapter -v -i shared_data_types.h protocol.xml
echo protocol.xml done.

qdbusxml2cpp -v -a channel_adapter -v -i shared_data_types.h -l QGVChannel channel.xml
echo channel.xml done.
qdbusxml2cpp -v -a channel_type_text_adapter -v -i shared_data_types.h channel.type.text.xml
echo channel.type.text.xml done.
qdbusxml2cpp -v -a channel_i_msgs_adapter -v -i shared_data_types.h channel.i.messages.xml
echo channel.i.messages.xml done.

mv *_adapter* gen

