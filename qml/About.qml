import Qt 4.7
import "helper.js" as Code

Item {
    id: container

    signal sigBack()
    signal sigLinkActivated(string strLink)

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

}//Item(container)
