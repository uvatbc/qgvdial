import Qt 4.7

Rectangle {
    id: wMainView

    property string layoutName: "maemo-portrait"
    states: [
        State {
            name: "desktop"; when: (layoutName == "desktop")
            PropertyChanges {
                target: wMainView
                width: 390; height: 170
            }
        },
        State {
            name: "maemo-portrait"; when: (layoutName == "maemo-portrait")
            PropertyChanges {
                target: wMainView
                width: 800; height: 400
            }
        }
    ]//states

    Flow {
        anchors.fill: parent
        spacing: (wMainView.layoutName=="desktop"?2:4)

        DialDisp {
            id: wDisp
            layoutName: wMainView.layoutName
        }//DialDisp

        Keypad {
            layoutName: wMainView.layoutName
            onBtnClick: {
                wDisp.strNum += strText
            }
        }//Keypad
    }//Flow
}//Rectangle
