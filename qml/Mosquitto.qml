/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

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
import "helper.js" as Code

Item {
    id: container
    objectName: "MosquittoPage"

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
        anchors.fill: parent
        anchors.topMargin: 2
        spacing: 2

        RadioButton {
            id: mqSupport
            width: parent.width
            pixelSize: (container.height + container.width) / 30

            text: "Enable mosquitto support"
        }// RadioButton (mqSupport)

        Row {
            width: parent.width
            spacing: 2
            opacity: (bEnable ? 1 : 0)

            Text {
                id: lblHost
                text: "Host:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: (container.height + container.width) / 30
            }

            MyTextEdit {
                id: textMqServer
                width: parent.width - lblHost.width
                anchors.verticalCenter: parent.verticalCenter
                text: "mosquitto.example.com"
                pixelSize: (container.height + container.width) / 30
                KeyNavigation.tab: textMqPort
                KeyNavigation.backtab: textMqTopic
            }
        }// Row (Mq server)

        Row {
            width: parent.width
            spacing: 2

            opacity: (bEnable ? 1 : 0)

            Text {
                id: lblPort
                text: "Port:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: (container.height + container.width) / 30
            }

            MyTextEdit {
                id: textMqPort
                width: parent.width - lblPort.width
                anchors.verticalCenter: parent.verticalCenter
                text: "1883"
                validator: IntValidator { bottom: 0; top: 65535 }
                pixelSize: (container.height + container.width) / 30
                KeyNavigation.tab: textMqTopic
                KeyNavigation.backtab: textMqServer
            }
        }// Row (Mq port)

        Row {
            width: parent.width
            spacing: 2
            opacity: (bEnable ? 1 : 0)

            Text {
                id: lblTopic
                text: "Topic to sub:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: (container.height + container.width) / 30
            }

            MyTextEdit {
                id: textMqTopic
                width: parent.width - lblTopic.width
                anchors.verticalCenter: parent.verticalCenter
                text: "gv_notify"
                pixelSize: (container.height + container.width) / 30
                KeyNavigation.tab: textMqServer
                KeyNavigation.backtab: textMqPort
            }
        }// Row (Mq topic to subscribe to)

        Row {
            width: parent.width
            spacing: 1

            MyButton {
                mainText: "Save"
                width: (parent.width / 2) - parent.spacing
                mainPixelSize: (container.height + container.width) / 30

                onClicked: {
                    container.sigMosquittoChanges (bEnable,
                                                   textMqServer.text,
                                                   textMqPort.text,
                                                   textMqTopic.text);
                    container.sigDone(true);
                }

            }//MyButton (Save)

            MyButton {
                mainText: "Cancel"
                width: (parent.width / 2) - parent.spacing
                mainPixelSize: (container.height + container.width) / 30

                onClicked: container.sigDone(false);
            }//MyButton (Cancel)
        }// Save and cancel buttons
    }// Column
}// Item (top level)
