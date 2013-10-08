/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

import bb.cascades 1.0

TabbedPane {
    id: container
    objectName: "MainTabbedPane"
    
    showTabsOnActionBar: true
    Tab {
        id: dialTab
        title: qsTr("Dialpad")
        DialPage {
            id: dialPage
        }
    }//DialPage
    Tab {
        id: contactsTab
        title: qsTr("Contacts")
        ContactsPage {
            id: contactsPage
        }
    }//ContactsPage
    Tab {
        id: inboxTab
        title: qsTr("Inbox")
        InboxPage {
            id: inboxPage
        }
    }//InboxPage
    Tab {
        id: settingsTab
        title: qsTr("Settings")
        SettingsPage {
            id: settingsPage
        }
    }//SettingsPage tab
    
    function showTab(index) {
        if ((index < 0) || (index > 3)) {
            console.debug("Array index out of bounds for tab selector");
            return;
        }
        
        container.activeTab = container.at(index);
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
