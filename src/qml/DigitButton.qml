import Qt 4.7
import "helper.js" as Code

Item {
    id: digButton
    width: ((parent.width / layoutGrid.columns) - parent.spacing);
    height: ((parent.height / layoutGrid.rows) - parent.spacing);

    // Main text in the button. "0" "1" ...
    property string mainText: "2"
    // Subtext
    property string subText: "abc"
    // Is this a deletion button?
    property bool isDel: false

    // Button emits clicks, but we also mention what is the text to display
    signal clicked(string strText)
    // Button can also emit a delete
    signal delClicked()

    Rectangle {
        id: rectangle
        anchors.fill: parent
        border.color: "black"
        smooth: true
        radius: ((height + width) / 20);

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
            text: mainText
            color: "black"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: -5
            font.pointSize: Code.btnFontPoint ();
            font.bold: true
        }// Text

        // The sub text: text
        Text {
            id: sText
            text: subText
            color: "darkgrey"
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            font.pointSize: Code.btnSubTextFontPoint ();
        }// Text

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            onClicked: (isDel?container.delClicked:digButton.clicked(mainText));

            onPressed: rectangle.gradient = grad2
            onReleased: rectangle.gradient = grad1
        }// MouseArea
    }// Rectangle
}// Item
