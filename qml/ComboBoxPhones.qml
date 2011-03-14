import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: listOfPhones
    color: "black"

    signal selectionChanged(int iIndex, string name)

    ListView {
        id: listView
        anchors.fill: parent
        clip: true

        model: g_registeredPhonesModel
        delegate: Rectangle {
            id: rectDelegate
            color: "black"
            border.color: "grey"
            width: listView.width
            height: listView.height / 3

            Column {
                anchors.fill:  parent
                spacing: 2
                Text {
                    id: txtName
                    text: name
                    color: "white"
                    width: listView.width
                    font.pixelSize: (rectDelegate.height / 2) - 2
                    elide: Text.ElideMiddle
                }

                Text {
                    id: txtNumber
                    text: description
                    color: "white"
                    width: listView.width
                    font.pixelSize: (rectDelegate.height / 2) - 2
                    elide: Text.ElideMiddle
                }
            }// Column

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    listOfPhones.selectionChanged(index, name);
                }
            }// MouseArea
        }// delegate Rectangle
    }//ListView
}//Component: listOfPhones
