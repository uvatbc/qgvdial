import Qt 4.7

Rectangle {
    id: wDialer
    color: "white"

    signal btnClick(string strText)

    property string layoutName: "desktop"
    states: [
        State {
            name: "desktop"; when: (layoutName == "desktop")
            PropertyChanges {
                target: wDialer
                width: 190; height: 170
            }
        },
        State {
            name: "maemo-portrait"; when: (layoutName == "maemo-portrait")
            PropertyChanges {
                target: wDialer
                width: 420; height: 400
            }
        }
    ]//states

    Grid {
        anchors.fill: parent
        rows: 4; columns: 3
        spacing: (wDialer.layoutName == "desktop"?2:4)

        DigitButton { mainText: "1"; subText: ""
            layoutName: wDialer.layoutName
            onClicked: btnClick(strText); }
        DigitButton { mainText: "2"; subText: "ABC"
            layoutName: wDialer.layoutName
            onClicked: btnClick(strText); }
        DigitButton { mainText: "3"; subText: "DEF"
            layoutName: wDialer.layoutName
            onClicked: btnClick(strText); }
        DigitButton { mainText: "4"; subText: "GHI"
            layoutName: wDialer.layoutName
            onClicked: btnClick(strText); }
        DigitButton { mainText: "5"; subText: "JKL"
            layoutName: wDialer.layoutName
            onClicked: btnClick(strText); }
        DigitButton { mainText: "6"; subText: "MNO"
            layoutName: wDialer.layoutName
            onClicked: btnClick(strText); }
        DigitButton { mainText: "7"; subText: "PQRS"
            layoutName: wDialer.layoutName
            onClicked: btnClick(strText); }
        DigitButton { mainText: "8"; subText: "TUV"
            layoutName: wDialer.layoutName
            onClicked: btnClick(strText); }
        DigitButton { mainText: "9"; subText: "WXYZ"
            layoutName: wDialer.layoutName
            onClicked: btnClick(strText); }
        DigitButton { mainText: "+"; subText: ""
            layoutName: wDialer.layoutName
            onClicked: btnClick(strText); }
        DigitButton { mainText: "0"; subText: ""
            layoutName: wDialer.layoutName
            onClicked: btnClick(strText); }
        DigitButton { mainText: "#"; subText: ""
            layoutName: wDialer.layoutName
            onClicked: btnClick(strText); }
    }// Grid
}// Rectangle
