import Qt 4.7

Rectangle {
    signal sigCall
    signal sigText
    signal sigDel

    Row {
        anchors.fill: parent
        spacing: 1

        Rectangle {
            id: btnCall
            width: parent.width * (1 / 3)
            height: parent.height
            radius: ((height + width) / 20);
            color: "black"

            Image {
                anchors.fill: parent
                fillMode: Image.Stretch
                source: "Phone.png"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: sigCall()

                onPressed: btnCall.border.color = "grey"
                onReleased: btnCall.border.color = "black"
            }
        }

        Rectangle {
            id: btnText

            width: parent.width * (1 / 3)
            height: parent.height
            radius: ((height + width) / 20);
            color: "black"

            Image {
                anchors.fill: parent
                fillMode: Image.Stretch
                source: "SMS.png"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: sigText()

                onPressed: btnText.border.color = "grey"
                onReleased: btnText.border.color = "black"
            }
        }

        Rectangle {
            id: btnDel

            width: parent.width * (1 / 3)
            height: parent.height
            radius: ((height + width) / 20);
            color: "black"

            Image {
                anchors.fill: parent
                fillMode: Image.Stretch
                source: "left_arrow.png"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: sigDel()

                onPressed: btnDel.border.color = "grey"
                onReleased: btnDel.border.color = "black"
            }
        }
    }
}
