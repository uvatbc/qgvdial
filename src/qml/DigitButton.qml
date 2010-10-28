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

    MyButton {
        id: btn
        mainText: digButton.mainText

        onClicked: (digButton.isDel?digButton.delClicked():digButton.clicked(mainText));

        // The sub text: text
        Text {
            id: sText
            text: subText
            color: "darkgrey"
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            font.pointSize: Code.btnSubTextFontPoint ();
        }// Text
    }
}// Item
