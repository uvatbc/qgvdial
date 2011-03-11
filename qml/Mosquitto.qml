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
            fontPoint: Code.btnFontPoint()/10

            text: "Enable mosquitto support"
        }// RadioButton (mqSupport)

        Row {
            width: parent.width
            spacing: 2
            opacity: (bEnable ? 1 : 0)

            Text {
                text: "Host:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pointSize: Code.btnFontPoint()/10
            }

            TextInput {
                id: textMqServer
                anchors.verticalCenter: parent.verticalCenter
                text: "mosquitto.example.com"
                color: "white"
                font.pointSize: Code.btnFontPoint()/10
            }
        }// Row (Mq server)

        Row {
            width: parent.width
            spacing: 2

            opacity: (bEnable ? 1 : 0)

            Text {
                text: "Port:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pointSize: Code.btnFontPoint()/10
            }

            TextInput {
                id: textMqPort
                anchors.verticalCenter: parent.verticalCenter
                text: "1883"
                color: "white"
                validator: IntValidator { bottom: 0; top: 65535 }
                font.pointSize: Code.btnFontPoint()/10
            }
        }// Row (Mq port)

        Row {
            width: parent.width
            spacing: 2
            opacity: (bEnable ? 1 : 0)

            Text {
                text: "Topic to sub:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pointSize: Code.btnFontPoint()/10
            }

            TextInput {
                id: textMqTopic
                anchors.verticalCenter: parent.verticalCenter
                text: "gv_notify"
                color: "white"
                font.pointSize: Code.btnFontPoint()/10
            }
        }// Row (Mq topic to subscribe to)

        Row {
            width: parent.width
            spacing: 1

            MyButton {
                mainText: "Save"
                width: (parent.width / 2) - parent.spacing
                mainPixelSize: container.height / 10

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
                mainPixelSize: container.height / 10

                onClicked: container.sigDone(false);
            }//MyButton (Cancel)
        }// Save and cancel buttons
    }// Column
}// Item (top level)
