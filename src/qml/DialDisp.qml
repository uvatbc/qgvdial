import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: wDisp
    // The number in the text box
    property string strNum: txtNum.text

    width: Code.calcFlowChildWidth();
    height: Code.calcFlowChildHeight();

    Column {
        anchors.fill: parent

        ComboBoxPhones {
            lstItems: ["a", "b"]
            currSelected: 0
        }

        TextEdit {
            id: txtNum
            color: "white"
            width: wDisp.width
            height: (wDisp.height * 60 / 400)
            textFormat: TextEdit.PlainText
            text: wDisp.strNum
            font {
                pointSize: Code.btnFontPoint();
                bold: true
            }
        }// TextEdit
    }// Flow
}// Rectangle
