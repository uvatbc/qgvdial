import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: main
    width: 250; height: 400
    property bool landscape: main.width > main.height

    signal sigCall(string strNumber)
    signal sigText(string strNumber)
    signal sigContacts
    signal sigInbox
    signal sigSelChanged (int index)
    signal sigNumChanged (string strNumber)

    MainView_l {
        opacity: (landscape?1:0)
        anchors.fill: parent

        onSigCall: main.sigCall()
        onSigText: main.sigText()
        onSigContacts: main.sigContacts()
        onSigInbox: main.sigInbox()
        onSigSelChanged: main.sigSelChanged(index)
        onSigNumChanged: main.sigNumChanged(strNumber)
    }

    MainView_p {
        opacity: (landscape?0:1)
        anchors.fill: parent

        onSigCall: main.sigCall()
        onSigText: main.sigText()
        onSigContacts: main.sigContacts()
        onSigInbox: main.sigInbox()
        onSigSelChanged: main.sigSelChanged(index)
        onSigNumChanged: main.sigNumChanged(strNumber)
    }
}
