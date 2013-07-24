// Tabbed Pane project template
import bb.cascades 1.0

TabbedPane {
    showTabsOnActionBar: true
    Tab {
        title: qsTr("Dialpad")
        DialPage {
            id: dialPage
        }
    }//DialPage
    Tab {
        title: qsTr("Contacts")
        ContactsPage {
            id: contactsPage
        }
    }//ContactsPage
    Tab {
        title: qsTr("Inbox")
        InboxPage {
            id: inboxPage
        }
    }//InboxPage
    Tab {
        title: qsTr("Settings")
        SettingsPage {
            id: settingsPage
        }
    }//SettingsPage tab

    onCreationCompleted: {
        // this slot is called when declarative scene is created
        // write post creation initialization here
        console.log("TabbedPane - onCreationCompleted()")

        // enable layout to adapt to the device rotation
        // don't forget to enable screen rotation in bar-bescriptor.xml (Application->Orientation->Auto-orient)
        OrientationSupport.supportedDisplayOrientation = SupportedDisplayOrientation.All;
    }

}
