import Qt 4.7

Rectangle {
    id: wDisp
    property string strNum: txtNum.text

    property string layoutName: "desktop"
    states: [
        State {
            name: "desktop"; when: (layoutName == "desktop")
            PropertyChanges {
                target: wDisp
                width: 200; height: 30
            }
        },
        State {
            name: "maemo-portrait"; when: (layoutName == "maemo-portrait")
            PropertyChanges {
                target: wDisp
                width: 365; height: 300
            }
        }
    ]

    Column {
        anchors.fill: parent

        ComboBoxPhones {
            lstItems: ["a", "b"]
            currSelected: 0
            layoutName: wDisp.layoutName
        }

        TextEdit {
            id: txtNum
            color: "white"
            width: wDisp.width
            height: (wDisp.layoutName=="desktop"?30:60)
            textFormat: TextEdit.PlainText
            text: wDisp.strNum
            font {
                pointSize: (wDisp.layoutName=="desktop"?14:28)
                bold: true
            }
        }// TextEdit
    }// Flow
}// Rectangle
