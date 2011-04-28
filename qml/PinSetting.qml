import Qt 4.7
import "helper.js" as Code

Item {
    id: container
    objectName: "PinSettingsPage"

    function setValues(bEnable, pin) {
        console.debug ("QML: Setting Pin settings")
        pinSupport.check = bEnable;
        textPin.text = port;
    }

    signal sigDone(bool bSave)
    signal sigPinSettingChanges(bool bEnable, string pin)
    signal sigMsgBoxDone (bool ok)

    property bool bEnable: pinSupport.check

    Column {
        anchors.fill: parent
        anchors.topMargin: 2
        spacing: 2

        RadioButton {
            id: pinSupport
            width: parent.width
            pixelSize: (container.height + container.width) / 30

            text: "Use PIN for GV dial"
        }// RadioButton (pinSupport)

        Row {
            width: parent.width
            spacing: 2

            opacity: (bEnable ? 1 : 0)

            Text {
                id: lblPin
                text: "Pin:"
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: (container.height + container.width) / 30
            }

            MyTextEdit {
                id: textPin
                width: parent.width - lblPin.width
                anchors.verticalCenter: parent.verticalCenter
                text: "0000"
                validator: IntValidator { bottom: 0; top: 9999 }
                pixelSize: (container.height + container.width) / 30
            }
        }// Row (Mq port)

        Row {
            width: parent.width
            spacing: 1

            MyButton {
                mainText: "Save"
                width: (parent.width / 2) - parent.spacing
                mainPixelSize: (container.height + container.width) / 30

                onClicked: {
                    container.sigPinSettingChanges (bEnable, textPin.text);
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

    MsgBox {
        id: msgBox
        opacity: ((container.opacity == 1 && g_bShowMsg == true) ? 1 : 0)
        msgText: g_strMsgText

        width: container.width - 20
        height: (container.width + container.height) / 6
        anchors.centerIn: container

        onSigMsgBoxOk: container.sigMsgBoxDone(true)
        onSigMsgBoxCancel: container.sigMsgBoxDone(false)
    }
}// Item (top level)
