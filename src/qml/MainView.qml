import Qt 4.7

Rectangle {
    id: wMainView
    color: "#00000000"

    property string layoutName: "desktop"
//    property string layoutName: "maemo-portrait"
    states: [
        State {
            name: "desktop"; when: (layoutName == "desktop")
            PropertyChanges {
                target: wMainView
                width: 250; height: 400
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
        spacing: (wMainView.layoutName=="desktop"? 2 : 4)

        DialDisp {
            id: wDisp
            color: wMainView.color
        }//DialDisp

        Keypad {
            color: wMainView.color
            onBtnClick: {
                wDisp.strNum += strText
            }
        }//Keypad
    }//Flow
}//Rectangle
