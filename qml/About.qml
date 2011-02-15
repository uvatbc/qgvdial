import Qt 4.7
import "helper.js" as Code

Item {
    id: container

    signal sigBack()
    signal sigLinkActivated(string strLink)

    Column {
        anchors.fill: parent
        anchors.topMargin: 2
        spacing: 2

        Text {
            text: "Version: __QGVDIAL_VERSION__"
            width: parent.width
            font.pointSize: Code.btnFontPoint()/10
            color: "white"
        }//Text (version)

        Text {
            text: "Project homepage is <a href=http://www.code.google.com/p/qgvdial>here</a>"
            width: parent.width
            font.pointSize: Code.btnFontPoint()/10
            color: "white"
            onLinkActivated: container.sigLinkActivated(link)
        }//Text (homepage)

        MyButton {
            mainText: "Back"
            width: parent.width
            mainFontPoint: Code.btnFontPoint()/8

            onClicked: container.sigBack();
        }//MyButton (Back)
    }//Column
}//Item(container)
