import bb.cascades 1.0

Container {
    id: container
    
    signal keyPress(string text)
    signal call
    signal text
    signal del

    layout: StackLayout { orientation: LayoutOrientation.TopToBottom }
    
    Container {
        layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
        layoutProperties: StackLayoutProperties { spaceQuota: 1.0 }

        KeyButton {
            mainText: "1"
            onClicked: container.keyPress(mainText);
        }
        KeyButton {
            mainText: "2"
            subText: "ABC"
            onClicked: container.keyPress(mainText);
        }
        KeyButton {
            mainText: "3"
            subText: "DEF"
            onClicked: container.keyPress(mainText);
        }
    }//Row 1

    Container {
        layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
        layoutProperties: StackLayoutProperties { spaceQuota: 1.0 }

        KeyButton {
            mainText: "4"
            subText: "GHI"
            onClicked: container.keyPress(mainText);
        }
        KeyButton {
            mainText: "5"
            subText: "JKL"
            onClicked: container.keyPress(mainText);
        }
        KeyButton {
            mainText: "6"
            subText: "MNO"
            onClicked: container.keyPress(mainText);
        }
    }//Row 2

    Container {
        layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
        layoutProperties: StackLayoutProperties { spaceQuota: 1.0 }

        KeyButton {
            mainText: "7"
            subText: "PQRS"
            onClicked: container.keyPress(mainText);
        }
        KeyButton {
            mainText: "8"
            subText: "TUV"
            onClicked: container.keyPress(mainText);
        }
        KeyButton {
            mainText: "9"
            subText: "WXYZ"
            onClicked: container.keyPress(mainText);
        }
    }//Row 3

    Container {
        layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
        layoutProperties: StackLayoutProperties { spaceQuota: 1.0 }

        KeyButton {
            mainText: "*"
            onClicked: container.keyPress(mainText);
        }
        KeyButton {
            mainText: "0"
            subText: "+"
            onClicked: container.keyPress(mainText);
            onPressAndHold: container.keyPress(subText);
        }
        KeyButton {
            mainText: "#"
            onClicked: container.keyPress(mainText);
        }
    }//Row 4

    Container {
        layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
        layoutProperties: StackLayoutProperties { spaceQuota: 1.0 }

        Button {
            text: "Call"
            onClicked: container.call();
        }
        Button {
            text: "Text"
            onClicked: container.text();
        }
        Button {
            text: "\u232B"
            onClicked: container.del();
        }
    }//Row 5
}//Container
