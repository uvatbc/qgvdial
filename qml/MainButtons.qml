import Qt 4.7
import "helper.js" as Code

Item {
    id: container

    anchors.fill: parent

    signal sigDialpad
    signal sigContacts
    signal sigInbox
    signal sigSettings

    TextButton {
        id: btnReference
        opacity: 0
        text: "Contacts"
    }

    ListView {
        id: mainBtnList
        anchors.fill: parent
        clip: true

        model: ListModel {
            ListElement {
                name: "Dialpad"
                imagePath: "Phone.png"
            }
            ListElement {
                name: "Contacts"
                imagePath: "users.png"
            }
            ListElement {
                name: "Inbox"
                imagePath: "note.png"
            }
            ListElement {
                name: "Settings"
                imagePath: "Google.png"
            }
        }

        delegate: MyButton {
            mainText: name

            id: btn
            height: mainBtnList.height / (mainBtnList.model.count + mainBtnList.spacing)
            width: mainBtnList.width

            mainFontPoint: Code.btnFontPoint()/3.7

            onClicked: {
                if (name == "Dialpad") {
                    container.sigDialpad();
                } else if (name == "Contacts") {
                    container.sigContacts();
                } else if (name == "Inbox") {
                    container.sigInbox();
                } else if (name == "Settings") {
                    container.sigSettings();
                }
            }//onClicked
        }//MyButton (delegate)
    }//ListView
}//Item (main container)
