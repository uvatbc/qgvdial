import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: wDisp

    // Expose the text edit as a property
    property TextEdit tEd: txtNum

    width: Code.calcFlowChildWidth();
    height: Code.calcFlowChildHeight();

    Column {
        anchors.fill: parent

        ComboBoxPhones {
//            lstItems: ["a", "b"]
            currSelected: 0
        }

        TextEdit {
            id: txtNum

            color: "white"
            width: wDisp.width
            height: (wDisp.height * 60 / 400)
            textFormat: TextEdit.PlainText
            cursorVisible: true
            wrapMode: TextEdit.WrapAnywhere
            selectByMouse: true
            font {
                pointSize: (Code.btnFontPoint()/3);
                bold: true
            }
        }// TextEdit
    }// Flow
}// Rectangle
