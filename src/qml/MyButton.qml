import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: button
    anchors.fill: parent
    border.color: "black"
    smooth: true
    radius: ((height + width) / 20);

    // Main text in the button
    property string mainText: "2"
    property Text someText: mText

    // Button emits clicks, but we also mention what is the text to display
    signal clicked(string strText)

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

    // The main text
    Text {
        id: mText
        text: button.mainText
        color: "black"

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -5

        font.pointSize: Code.btnFontPoint ();
        font.bold: true
    }// Text

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        onPressed: button.gradient = grad2;
        onReleased: button.gradient = grad1;

        onClicked: (button.clicked(mainText));
    }// MouseArea
}// Rectangle
