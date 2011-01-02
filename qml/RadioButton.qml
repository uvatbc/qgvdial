import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: container

    height: textLabel.height + 5
    width: textLabel.width + 5
    color: "black"

    property string text: "Yes or no question?"
    property bool check: false
    property real fontPoint: 40 // Outrageously large so that you are forced to change it.

    Row {
        anchors.fill: parent
        spacing: 2

        Rectangle {
            id: imageRect

            color: "white"
            border.color: "black"

            anchors.verticalCenter: parent.verticalCenter
            height: textLabel.height
            width: height

            Image {
                id: imageTick
                source: "tick.png"

                anchors.fill: parent

                opacity: (container.check == true ? 1 : 0)
            }
        }

        Text {
            id: textLabel
            text: container.text
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: container.fontPoint

            color: "white"
        }
    }

    MouseArea {
        anchors.fill: parent

        onClicked: {
            if (container.check == true) {
                container.check = false;
            } else {
                container.check = true;
            }
        }
    }

    states: [
        State {
            name: "pressed"
            PropertyChanges { target: container; color: "orange"}
        }

    ]
}
