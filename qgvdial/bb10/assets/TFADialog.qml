import bb.cascades 1.0

Page {
    id: container
    objectName: "TFADialog"
    
    property bool accepted: false
    property string pin

    signal done

    Container {
        Label {
            horizontalAlignment: HorizontalAlignment.Center
            text: "Enter two factor authentication PIN"
        }
        TextField {
            id: textPin
            hintText: "PIN"
        }
        Button {
            horizontalAlignment: HorizontalAlignment.Center
            text: "OK"
            onClicked: {
                container.accepted = true;
                container.pin = textPin.text;
                container.done();
            }
        }
    }
}
