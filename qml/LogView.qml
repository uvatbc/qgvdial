import Qt 4.7

Item {
    id: container

    signal sigBack()

    Column {
        id: mainColumn
        anchors.fill: parent
        anchors.topMargin: 2
        spacing: 1

        property int pixDiv: 15
        property int pixHeight: (container.height + container.width) / 2
        property int pixSize: pixHeight / pixDiv

        ListView {
            height: container.height - btnBack.height - mainColumn.spacing
            width: parent.width
            model: g_logModel
            clip: true

            delegate: Rectangle {
                height: logText.height + 2
                width: mainColumn.width
                color: "black"
                Text {
                    id: logText
                    text: modelData
                    color: "white"
                    width: parent.width
                    wrapMode: Text.Wrap
                }
            }
        }

        MyButton {
            id: btnBack

            mainText: "Back"
            width: parent.width
            mainPixelSize: mainColumn.pixSize

            onClicked: container.sigBack();
        }//MyButton (Back)
    }//Column
}//Item (container)
