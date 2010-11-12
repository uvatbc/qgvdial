import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: wDisp

    signal sigSelChanged (int index)

    // Expose the text edit as a property
    property alias txtEd: txtNum

    function slotSelectionChanged(index, name) {
        btnPhones.mainText = name;
        wDisp.sigSelChanged(index);
    }
    function slotCbBoxDestroy () {
        Code.cbBox.destroy ();
        Code.cbBox = null;
    }

    Column {
        anchors.fill: parent

        Rectangle {
            color: wDisp.color
            width: parent.width
            height: (parent.height / 5)

            MyButton {
                id: btnPhones
                mainText: currentPhoneName;
                anchors.fill: parent
                radius: ((height / 10.0) + (width / 60.0))

                onClicked: {
                    if (Code.compCbBox == null) {
                        Code.compCbBox = Qt.createComponent("ComboBoxPhones.qml");
                    }

                    if (Code.cbBox == null) {
                        Code.cbBox = Code.compCbBox.createObject(wDisp);
                        Code.cbBox.width  = btnPhones.width;
                        Code.cbBox.height = wDisp.height - btnPhones.height;

                        Code.cbBox.y = btnPhones.height;

                        Code.cbBox.selectionChanged.connect(slotSelectionChanged);
                        Code.cbBox.sigDestructor.connect(slotCbBoxDestroy);
                        Code.cbBox.alive = true;
                    } else {
                        Code.cbBox.killSelf ();
                    }
                }
            }
        }

        TextEdit {
            id: txtNum
            width: parent.width

            color: "white"
            textFormat: TextEdit.PlainText
            cursorVisible: true
            wrapMode: TextEdit.WrapAnywhere
            selectByMouse: true
            font {
                pointSize: (Code.btnFontPoint()/3);
                bold: true
            }

            height: (parent * 4 / 5)
        }// TextEdit
    }// Flow
}// Rectangle
