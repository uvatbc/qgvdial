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

NavigationPane {
    id: container
    
    Page {
        content: ListView {
            id: settingsListView
            objectName: "SettingsList"
            
            signal sigShowProxy
            
            onTriggered: {
                if (indexPath == 0) {
                    container.push(loginDetails);
                } else if (indexPath == 1) {
                    console.debug("Proxy settings please!");
                    settingsListView.sigShowProxy();
                } 
            }
            function showTfaDialog() {
                tfaDialog.pin = "";
                tfaDialog.accepted = false;
                container.push(tfaDialog);
            }
            function showAppPwDialog() {
                appPwDialog.appPw = "";
                appPwDialog.accepted = false;
                container.push(appPwDialog);
            }
            function showMessage(msg) {
                messagePage.message = msg;
                container.push(messagePage)
            }
            function showProxyPage(bEnable, bUseSystemProxy, host, port, bRequiresAuth, user, pass) {
                proxyDetails.setValues(bEnable, bUseSystemProxy, host, port, bRequiresAuth, user, pass);
                container.push(proxyDetails);
            }

            dataModel: ArrayDataModel {
                id: settingsModel
            }
            
            onCreationCompleted: {
                settingsModel.append("Login details");
                settingsModel.append("Proxy details");
            }
            
            listItemComponents: [
                ListItemComponent {
                    Label {
                        text: ListItem.data
                        horizontalAlignment: HorizontalAlignment.Center
                        textStyle { base: tsdxlarge.style }
                        
                        attachedObjects: [
                            TextStyleDefinition {
                                id: tsdxlarge
                                base: SystemDefaults.TextStyles.BodyText
                                fontSize: FontSize.XLarge
                            }
                        ]
                    }//Button
                }//ListItemComponent
            ]//listItemComponents
        }//ListView
    }//Page
    
    attachedObjects: [
        LoginDetails {
            id: loginDetails
            onDone: container.pop();
        },
        ProxyPage {
            id: proxyDetails
            onDone: container.pop();
        },
        TFADialog {
            id: tfaDialog
            onDone: container.pop()
        },
        AppPwDialog {
            id: appPwDialog
            onDone: container.pop()
        },
        MessagePage {
            id: messagePage
        }
    ]
}//NavigationPane
