import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: main
    width: 250; height: 400

    signal sigCall(string number)
    signal sigText(string number)
    signal sigSelChanged (int index)

    property bool landscape: main.width > main.height

    MainView_l {
        id: m_l

        opacity: (landscape?1:0)
        anchors.fill: parent

        onSigCall: main.sigCall(number)
        onSigText: main.sigText(number)
        onSigSelChanged: main.sigSelChanged(index)
        onSigNumChanged: m_p.theNumber = strNumber
    }

    MainView_p {
        id: m_p

        opacity: (landscape?0:1)
        anchors.fill: parent

        onSigCall: main.sigCall(number)
        onSigText: main.sigText(number)
        onSigSelChanged: main.sigSelChanged(index)
        onSigNumChanged: m_l.theNumber = strNumber
    }
}
