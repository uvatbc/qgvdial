import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: wDialer
    color: "white"
    width: Code.calcFlowChildWidth();
    height: Code.calcFlowChildHeight();

    signal btnClick(string strText)

    Grid {
        id: layoutGrid
        anchors.fill: parent
        rows: 4; columns: 3
        spacing: wDialer.parent.spacing

        DigitButton { mainText: "1"; subText: ""
            onClicked: btnClick(strText); }
        DigitButton { mainText: "2"; subText: "ABC"
            onClicked: btnClick(strText); }
        DigitButton { mainText: "3"; subText: "DEF"
            onClicked: btnClick(strText); }
        DigitButton { mainText: "4"; subText: "GHI"
            onClicked: btnClick(strText); }
        DigitButton { mainText: "5"; subText: "JKL"
            onClicked: btnClick(strText); }
        DigitButton { mainText: "6"; subText: "MNO"
            onClicked: btnClick(strText); }
        DigitButton { mainText: "7"; subText: "PQRS"
            onClicked: btnClick(strText); }
        DigitButton { mainText: "8"; subText: "TUV"
            onClicked: btnClick(strText); }
        DigitButton { mainText: "9"; subText: "WXYZ"
            onClicked: btnClick(strText); }
        DigitButton { mainText: "+"; subText: ""
            onClicked: btnClick(strText); }
        DigitButton { mainText: "0"; subText: ""
            onClicked: btnClick(strText); }
        DigitButton { mainText: "#"; subText: ""
            onClicked: btnClick(strText); }
    }// Grid
}// Rectangle
