import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: window
    color: "black"
    width: 400; height: 250

    signal sigCall(string strNumber)
    signal sigText(string strNumber)
    signal sigContacts
    signal sigInbox
    signal sigSelChanged (int index)
    signal sigNumChanged (string strNumber)

    property bool landscape: window.width > window.height
    property variant rotationDelta: landscape? -90 : 0

    // initial state is portrait
    property real baseWidth: landscape ? window.height : window.width
    property real baseHeight: landscape ? window.width : window.height

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
                onSigContacts: window.sigContacts()
                onSigInbox: window.sigInbox()

                onSigDel: Code.doDel()
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
