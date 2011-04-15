import Qt 4.7
import "helper.js" as Code

Item {
    id: container

    signal sigBack()
    signal sigLinkActivated(string strLink)
    signal sigMsgBoxDone (bool ok)

    Column {
        id: mainColumn
        anchors.fill: parent
        anchors.topMargin: 2
        spacing: 2

        property int pixDiv: 15
        property int pixHeight: (container.height + container.width) / 2
        property int pixSize: pixHeight / pixDiv

        Text {
            text: "Version: __QGVDIAL_VERSION__"
            width: parent.width
            font.pixelSize: mainColumn.pixSize
            color: "white"
        }//Text (version)

        Text {
            text: "<a href=http://www.code.google.com/p/qgvdial>Project homepage</a>"
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: mainColumn.pixSize
            color: "white"
            onLinkActivated: container.sigLinkActivated(link)
        }//Text (homepage)

        Text {
            text: "<a href=http://code.google.com/p/qgvdial/wiki/Changelog>Changelog</a>"
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: mainColumn.pixSize
            color: "white"
            onLinkActivated: container.sigLinkActivated(link)
        }//Text (Changelog)

        Text {
            text: "<a href=http://code.google.com/p/qgvdial/w/list>Wiki</a>"
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: mainColumn.pixSize
            color: "white"
            onLinkActivated: container.sigLinkActivated(link)
        }//Text (wiki)

        MyButton {
            mainText: "Back"
            width: parent.width
            mainPixelSize: mainColumn.pixSize

            onClicked: container.sigBack();
        }//MyButton (Back)
    }//Column

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
}//Item(container)
