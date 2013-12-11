import QtQuick 1.0

Item {
    id: container

    property int minimumValue: 0
    property int maximumValue: 100
    property int value: 0

    width: 250; height: 23
    clip: true

    BorderImage {
        source: "qrc:/progress-bar-background.png"
        width: parent.width; height: parent.height
        border { left: 4; top: 4; right: 4; bottom: 4 }

        BorderImage {
            id: highlight

            property int widthDest: ((container.width * (value - minimumValue)) / (maximumValue - minimumValue) - 6)
            width: highlight.widthDest

            source: "qrc:/progress-bar-bar.png"
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom; margins: 3 }

            border { left: 4; top: 4; right: 4; bottom: 4 }

            onWidthDestChanged: percentText.positionLabel();
        }
    }//Outer BorderImage

    Text {
        id: percentText

        anchors {
            verticalCenter: parent.verticalCenter
            right: parent.right
            rightMargin: positionLabel();
        }

        text: {
            var rv = Math.round(((value - minimumValue) / (maximumValue - minimumValue)) * 100);
            return rv.toString() + "%";
        }

        font.pixelSize: parent.height - 6

        function positionLabel() {
            if (percentText.width < (highlight.width + 4)) {
                percentText.color = "white";
                return parent.width - highlight.width;
            } else {
                percentText.color = "black";
                return parent.width - highlight.width - percentText.width - 4;
            }
        }
    }//Miles Text
}//Item
