import Qt 4.7

Item {
    id: cbNumbers
    width: 200; height: 30

    // List of items
//    property list<string> lstItems
    property variant lstItems
    // Currently selected item
    property int currSelected: -1


    Rectangle {
        id: rectangle
        anchors.fill: parent
        border.color: "black"
        smooth: true
        radius: 6.0

        // Two gradients: grad1 for pressed and grad2 for released
        Gradient {
            id: grad1
            GradientStop { position: 0.0; color: "#f6f7fa" }
            GradientStop { position: 1.0; color: "#0a0b0e" }
        }// Gradient
        Gradient {
            id: grad2
            GradientStop { position: 0.0; color: "#0a0b0e" }
            GradientStop { position: 1.0; color: "#f6f7fa" }
        }// Gradient

        gradient: grad1

        // The main text: numbers
        Text {
            id: mText
            text: (currSelected==-1?"...":lstItems[currSelected])
            color: "black"
            font { pointSize: 12; bold: true;  }
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }// Text

        MouseArea {
            id: mouseArea
            anchors.fill: parent

            onPressed: rectangle.gradient = grad2
            onReleased: rectangle.gradient = grad1
        }// MouseArea
    }// Rectangle
}// Item
