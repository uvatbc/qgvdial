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

    signal sigDone(bool bSave)
    signal sigRefreshChanges(bool bRefreshEnable, string minPeriod, string maxPeriod,
                             bool bMqEnable, string host, int port, string topic)
    signal sigRefreshPeriodSettings

    function setMqValues(bEnable, host, port, topic) {
        console.debug ("QML: Setting Mq settings")
        mqSupport.check = bEnable;
        textMqServer.text = host;
        textMqPort.text = port;
        textMqTopic.text = topic;
    }

    function setRefreshValues(bEnable, minPeriod, maxPeriod) {
        console.debug ("QML: Setting Refresh settings")
        periodicRefresh.check = bEnable;
        textMinRefreshPeriod.text = minPeriod;
        textMaxRefreshPeriod.text = maxPeriod;
    }

    height: mainColumn.height

    Column {
        id: mainColumn
        anchors {
            top: parent.top
            left: parent.left
        }
        spacing: 2
        width: parent.width

        // Internal property
        property real subTextPointSize: (8 * g_fontMul)

        RadioButton {
            id: periodicRefresh
            width: parent.width

            text: "Enable periodic refresh"

            pointSize: (8 * g_fontMul)
        }//RadioButton (enable periodicRefresh)

        Row {
            width: parent.width
            height: lblMinRefreshPeriod.height
            spacing: 2

            opacity: (periodicRefresh.check ? 1 : 0)

            Text {
                id: lblMinRefreshPeriod
                text: "Min period in sec:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font { family: "Nokia Sans"; pointSize: mainColumn.subTextPointSize }
                height: paintedHeight + 2
            }

            MyTextEdit {
                id: textMinRefreshPeriod
                width: parent.width - lblMinRefreshPeriod.width
                height: lblMinRefreshPeriod.height
                anchors.verticalCenter: parent.verticalCenter
                text: "30"
                validator: IntValidator { bottom: 0; top: 3600 }
                KeyNavigation.tab: textMaxRefreshPeriod
                KeyNavigation.backtab: textMaxRefreshPeriod
                pointSize: mainColumn.subTextPointSize
            }
        }// Row (Minimum period)

        Row {
            width: parent.width
            height: lblMaxRefreshPeriod.height
            spacing: 2

            opacity: (periodicRefresh.check ? 1 : 0)

            Text {
                id: lblMaxRefreshPeriod
                text: "Max period in sec:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font { family: "Nokia Sans"; pointSize: mainColumn.subTextPointSize }
                height: paintedHeight + 2
            }

            MyTextEdit {
                id: textMaxRefreshPeriod
                width: parent.width - lblMaxRefreshPeriod.width
                height: lblMaxRefreshPeriod.height
                anchors.verticalCenter: parent.verticalCenter
                text: "300"
                validator: IntValidator { bottom: 0; top: 3600 }
                KeyNavigation.tab: textMinRefreshPeriod
                KeyNavigation.backtab: textMinRefreshPeriod
                pointSize: mainColumn.subTextPointSize
            }
        }// Row (Maximum period)

        RadioButton {
            id: mqSupport
            width: parent.width

            text: "Enable mosquitto"
            pointSize: mainColumn.subTextPointSize
        }// RadioButton (enable mqSupport)

        Row {
            width: parent.width
            height: lblHost.height
            spacing: 2
            opacity: (mqSupport.check ? 1 : 0)

            Text {
                id: lblHost
                text: "Host:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font { family: "Nokia Sans"; pointSize: mainColumn.subTextPointSize }
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
                pointSize: mainColumn.subTextPointSize
            }
        }// Row (Mq server)

        Row {
            width: parent.width
            height: lblPort.height
            spacing: 2

            opacity: (mqSupport.check ? 1 : 0)

            Text {
                id: lblPort
                text: "Port:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font { family: "Nokia Sans"; pointSize: mainColumn.subTextPointSize }
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
                pointSize: mainColumn.subTextPointSize
            }
        }// Row (Mq port)

        Row {
            width: parent.width
            height: lblTopic.height
            spacing: 2
            opacity: (mqSupport.check ? 1 : 0)

            Text {
                id: lblTopic
                text: "Topic to sub:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font { family: "Nokia Sans"; pointSize: mainColumn.subTextPointSize }
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
                pointSize: mainColumn.subTextPointSize
            }
        }// Row (Mq topic to subscribe to)

        SaveCancel {
            anchors {
                left: parent.left
                leftMargin: 1
            }
            width: parent.width - 1

            onSigSave: {
                container.sigRefreshChanges (periodicRefresh.check,
                                             textMinRefreshPeriod.text,
                                             textMaxRefreshPeriod.text,
                                             mqSupport.check,
                                             textMqServer.text,
                                             textMqPort.text,
                                             textMqTopic.text);
                container.sigDone(true);
            }

            onSigCancel: {
                container.sigRefreshPeriodSettings();
                container.sigDone(false);
            }
        }// Save and cancel buttons
    }// Column
}// Item (top level)
