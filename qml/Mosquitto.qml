/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

import Qt 4.7

Item {
    id: container
    objectName: "MosquittoPage"

    height: mainColumn.height + 2

    function setValues(bEnable, host, port, topic) {
        console.debug ("QML: Setting Mq settings")
        mqSupport.check = bEnable;
        textMqServer.text = host;
        textMqPort.text = port;
        textMqTopic.text = topic;
    }

    signal sigDone(bool bSave)
    signal sigMosquittoChanges(bool bEnable, string host, int port, string topic)

    property bool bEnable: mqSupport.check

    Column {
        id: mainColumn
        anchors {
            top: parent.top
            left: parent.left
        }
        spacing: 2
        width: parent.width

        RadioButton {
            id: mqSupport
            width: parent.width

            text: "Enable mosquitto"
        }// RadioButton (mqSupport)

        Row {
            width: parent.width
            height: lblHost.height
            spacing: 2
            opacity: (bEnable ? 1 : 0)

            Text {
                id: lblHost
                text: "Host:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font { family: "Nokia Sans"; pointSize: (10 * g_fontMul) }
                height: paintedHeight + 2
            }

            MyTextEdit {
                id: textMqServer
                width: parent.width - lblHost.width
                height: lblHost.height
                anchors.verticalCenter: parent.verticalCenter
                text: "mosquitto.example.com"
                KeyNavigation.tab: textMqPort
                KeyNavigation.backtab: textMqTopic
            }
        }// Row (Mq server)

        Row {
            width: parent.width
            height: lblPort.height
            spacing: 2

            opacity: (bEnable ? 1 : 0)

            Text {
                id: lblPort
                text: "Port:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font { family: "Nokia Sans"; pointSize: (10 * g_fontMul) }
                height: paintedHeight + 2
            }

            MyTextEdit {
                id: textMqPort
                width: parent.width - lblPort.width
                height: lblPort.height
                anchors.verticalCenter: parent.verticalCenter
                text: "1883"
                validator: IntValidator { bottom: 0; top: 65535 }
                KeyNavigation.tab: textMqTopic
                KeyNavigation.backtab: textMqServer
            }
        }// Row (Mq port)

        Row {
            width: parent.width
            height: lblTopic.height
            spacing: 2
            opacity: (bEnable ? 1 : 0)

            Text {
                id: lblTopic
                text: "Topic to sub:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font { family: "Nokia Sans"; pointSize: (10 * g_fontMul) }
                height: paintedHeight + 2
            }

            MyTextEdit {
                id: textMqTopic
                width: parent.width - lblTopic.width
                height: lblTopic.height
                anchors.verticalCenter: parent.verticalCenter
                text: "gv_notify"
                KeyNavigation.tab: textMqServer
                KeyNavigation.backtab: textMqPort
            }
        }// Row (Mq topic to subscribe to)

        SaveCancel {
            anchors {
                left: parent.left
                leftMargin: 1
            }
            width: parent.width - 1

            onSigSave: {
                container.sigMosquittoChanges (bEnable,
                                               textMqServer.text,
                                               textMqPort.text,
                                               textMqTopic.text);
                container.sigDone(true);
            }

            onSigCancel: container.sigDone(false);
        }// Save and cancel buttons
    }// Column
}// Item (top level)
