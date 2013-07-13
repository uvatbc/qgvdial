import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    tools: commonTools

    TextField {
        inputMethodHints: Qt.ImhDialableCharactersOnly
        placeholderText: "Enter number here"

        anchors {
            top: parent.top
            topMargin: 40
            horizontalCenter: parent.horizontalCenter
        }

        width: parent.width
    }
}
