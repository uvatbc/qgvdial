import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: main
    color: "black"

    signal sigCall(string number)
    signal sigText(string number)
    signal sigSelChanged (int index)
    signal sigMsgBoxDone (bool ok)

    property bool landscape: main.width > main.height
    property variant rotationDelta: landscape? -90 : 0

    // initial state is portrait
    property real baseWidth: landscape ? main.height : main.width
    property real baseHeight: landscape ? main.width : main.height
    property string theNumber: ""

    Flow {
        anchors.fill: parent

        Column {
            width: wDisp.width
            height: wDisp.height + (actBtn_l.height * actBtn_l.opacity)
            spacing: 0

            DialDisp {
                id: wDisp

                theNumber: main.theNumber

                color: main.color
                width: landscape ? main.width / 2 : main.width
                height: main.height * (landscape ? (3 / 4) : (7.5 / 18))

                onSigSelChanged: main.sigSelChanged (index)
                onSigNumChanged: {main.theNumber = strNumber;}
            }//DialDisp

            ActionButtons {
                id: actBtn_l
                color: main.color

                opacity: landscape ? 1 : 0

                width: wDisp.width
                height: wDisp.height / 3

                onSigCall: main.sigCall(wDisp.txtEd.text)
                onSigText: main.sigText(wDisp.txtEd.text)

                onSigDel: Code.doDel()
                onSigClear: wDisp.txtEd.text = ""
            }//ActionButtons (landscape mode - horizontal)
        }//Column

        Keypad {
            color: main.color
            width: parent.width / (landscape ? 2 : 1)
            height: parent.height * (landscape ? 1 : (8 / 18))

            onBtnClick: Code.doIns(strText)
            onBtnDelClick: Code.doDel()
        }//Keypad

        ActionButtons {
            color: main.color
            opacity: landscape ? 0 : 1

            width: main.width
            height: main.height * (2.5 / 18)

            onSigCall: main.sigCall(wDisp.txtEd.text)
            onSigText: main.sigText(wDisp.txtEd.text)

            onSigDel: Code.doDel()
            onSigClear: wDisp.txtEd.text = ""
        }//ActionButtons (portrait mode - vertical)
    }//Flow

    MsgBox {
        id: msgBox
        opacity: ((main.opacity == 1 && g_bShowMsg == true) ? 1 : 0)
        msgText: g_strMsgText

        width: main.width - 20
        height: (main.width + main.height) / 6
        anchors.centerIn: main

        onSigMsgBoxOk: main.sigMsgBoxDone(true)
        onSigMsgBoxCancel: main.sigMsgBoxDone(false)
    }
}
