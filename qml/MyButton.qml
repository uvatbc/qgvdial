import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: button
    border.color: "black"
    smooth: true

    radius: ((height + width) / 20);

    // Main text in the button
    property string mainText: "2"
    //property alias mainFontPoint: mText.font.pointSize
    property real mainFontPoint: Code.btnFontPoint()

    // Button emits clicks, but we also mention what is the text to display
    signal clicked(string strText)
    signal pressHold(string strText)

    gradient: Gradient {
        GradientStop { id: gradientStop; position: 0.0; color: "azure" }
        GradientStop { position: 1.0; color: palette.button }
    }
    SystemPalette { id: palette }

    // The main text
    Text {
        id: mText
        text: button.mainText
        color: "black"

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        font.pointSize: mainFontPoint
    }// Text

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        onClicked: button.clicked(mainText);
        onPressAndHold: button.pressHold(mainText);
    }// MouseArea

    states: State {
        name: "pressed"
        when: mouseArea.pressed
        PropertyChanges { target: gradientStop; color: "orange" }
    }
}// Rectangle
