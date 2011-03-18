import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: main

    signal sigCall(string number)
    signal sigText(string number)
    signal sigSelChanged (int index)
    signal sigMsgBoxDone (bool ok)

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

    MsgBox {
        id: msgBox
        opacity: ((main.opacity == 1 && g_bShowMsg == true) ? 1 : 0)
        msgText: g_strMsgText

        width: main.width - 20
        height: (main.width + main.height) / 6
        anchors.centerIn: main

        onSigMsgBoxOk: main.sigMsgBoxDone(true)
        onSigMsgBoxCancel: main.sigMsgBoxDone(false)
    }
}
