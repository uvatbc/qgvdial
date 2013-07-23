// Tabbed Pane project template
import bb.cascades 1.0

TabbedPane {
    showTabsOnActionBar: true
    Tab {
        title: qsTr("Dialpad")
        DialPage {
            id: dialPage
        }
    }
    Tab {
        title: qsTr("Contacts")
        ContactsPage {
            id: contactsPage
        }
    }
    Tab {
        title: qsTr("Inbox")
        InboxPage {
            id: inboxPage
        }
    }
    Tab {
        title: qsTr("Settings")
        SettingsPage {
            id: settingsPage
        }
    }

    onCreationCompleted: {
        // this slot is called when declarative scene is created
        // write post creation initialization here
        console.log("TabbedPane - onCreationCompleted()")

        // enable layout to adapt to the device rotation
        // don't forget to enable screen rotation in bar-bescriptor.xml (Application->Orientation->Auto-orient)
        OrientationSupport.supportedDisplayOrientation = SupportedDisplayOrientation.All;
    }

}
