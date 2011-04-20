import Qt 4.7

Rectangle {
    id:  container

    border.color: edHasFocus ? "orange" : "blue"
    color: "slategray"

    property string text: "You should have changed this text"
    //property alias text: textEd.text
    property alias echoMode: textEd.echoMode
    property bool edHasFocus: textEd.activeFocus
    property int pixelSize: 5000
    property alias validator: textEd.validator

    height: textEd.height

    signal sigTextChanged(string strText)
    onFocusChanged: {
        if (activeFocus == true) {
            focus = false;
            textEd.focus = true;
        }
    }

    TextInput {
        id: textEd
        anchors.verticalCenter: parent.verticalCenter
        text: container.text
        color: "white"
        width: container.width
        font.pixelSize: container.pixelSize

        onTextChanged: {
            container.sigTextChanged(text);
            container.text = textEd.text
        }
    }
}//Rectangle (around the email)
