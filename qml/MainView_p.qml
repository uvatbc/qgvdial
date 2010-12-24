import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: window
    color: "black"
    width: 250; height: 400

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

    Rectangle {
        color: "black"

        width: baseWidth
        height: baseHeight
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        rotation: rotationDelta

        Column {
            anchors.fill: parent
            width: baseWidth
            height: baseHeight

            DialDisp {
                id: wDisp
                color: window.color

                width: parent.width
                height: parent.height * (7.5 / 18)

                onSigSelChanged: window.sigSelChanged (index)
                onSigNumChanged: window.sigNumChanged (strNumber)
            }//DialDisp

            Keypad {
                color: window.color

                width: parent.width
                height: parent.height * (8 / 18)

                onBtnClick: Code.doIns(strText)
                onBtnDelClick: Code.doDel()
            }//Keypad

            ActionButtons {
                color: window.color

                width: parent.width
                height: parent.height * (2.5 / 18)

                onSigCall: window.sigCall(wDisp.txtEd.text)
                onSigText: window.sigText(wDisp.txtEd.text)

                onSigDel: Code.doDel()
            }
        }//Column
    }//Rectangle
}//Rectangle
