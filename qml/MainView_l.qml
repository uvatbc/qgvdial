import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: window
    color: "black"
    width: 400; height: 250

    signal sigCall(string number)
    signal sigText(string number)
    signal sigSelChanged (int index)
    signal sigNumChanged (string strNumber)

    property bool landscape: window.width > window.height
    property variant rotationDelta: landscape? -90 : 0

    // initial state is portrait
    property real baseWidth: landscape ? window.height : window.width
    property real baseHeight: landscape ? window.width : window.height
    property alias theNumber: wDisp.theNumber


    Row {
        id: main
        anchors.fill: parent

        Column {
            width: parent.width / 2
            height: parent.height

            DialDisp {
                id: wDisp

                color: window.color
                width: parent.width
                height: parent.height * (3 / 4)

                onSigSelChanged: window.sigSelChanged (index)
                onSigNumChanged: window.sigNumChanged (strNumber)
            }//DialDisp

            ActionButtons {
                color: window.color
                width: parent.width
                height: window.height * (1 / 4)

                onSigCall: window.sigCall(wDisp.txtEd.text)
                onSigText: window.sigText(wDisp.txtEd.text)

                onSigDel: Code.doDel()
                onSigClear: wDisp.txtEd.text = ""
            }
        }

        Keypad {
            color: window.color
            width: parent.width / 2
            height: parent.height

            onBtnClick: Code.doIns(strText)
            onBtnDelClick: Code.doDel()
        }//Keypad

    }//Flow
}//Rectangle
