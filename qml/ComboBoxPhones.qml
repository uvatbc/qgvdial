import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: listOfPhones
    color: "black"
    property bool alive: false

    signal selectionChanged(int iIndex, string name)
    signal sigHeightChanged(int iHeight)
    signal sigDestructor

    Timer {
        id: cbBoxCleanupTimer
        interval: 500
        onTriggered: listOfPhones.sigDestructor();
    }// Timer

    function killSelf () {
        listOfPhones.alive = false;
        cbBoxCleanupTimer.restart ();
    }

    states: [
        State{ name: "AliveState"; when: alive == true
            PropertyChanges { target: listView; opacity: 1 }
        },
        State{ name: "DeadState"; when: alive == false
            PropertyChanges { target: listView; opacity: 0 }
        }
    ]

    function calcFontPoint () {
        var pt = Math.max(listOfPhones.width,listOfPhones.height);
        pt = pt / 20;
        if (pt < 1) pt = 1;
        sigHeightChanged(pt);
        return pt;
    }

    ListView {
        id: listView
        anchors.fill: parent
        clip: true

        // Display smoothly
        Behavior on opacity {
            NumberAnimation { properties:"opacity"; duration: 400 }
        }

        model: registeredPhonesModel
        delegate: Rectangle {
            id: rectDelegate
            color: "black"
            border.color: "grey"
            width: listView.width
            height: (txtName.height + txtNumber.height)

            Column {
                Text {
                    id: txtName
                    text: name
                    color: "white"
                    width: listView.width
                    font.pointSize: calcFontPoint ();
                    elide: Text.ElideMiddle
                }

                Text {
                    id: txtNumber
                    text: description
                    color: "white"
                    width: listView.width
                    font.pointSize: calcFontPoint ();
                    elide: Text.ElideMiddle
                }
            }// Column

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    listOfPhones.selectionChanged(index, name);
                    killSelf ();
                }
            }// MouseArea
        }// delegate Rectangle
    }//ListView
}//Component: listOfPhones
