/*
 * qgvdial is a cross platform Google Voice Dialer
 * Copyright (C) 2009-2013  Yuvraaj Kelkar
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Contact: yuvraaj@gmail.com
 */

import bb.cascades 1.0

Page {
    id: container
    objectName: "ProxySettingsPage"

    function setValues(bEnable, bUseSystemProxy, host, port, bRequiresAuth, user, pass) {
        console.debug ("QML: Setting proxy settings");
        proxySupport.checked = bEnable;
        proxySystem.checked = bUseSystemProxy;
        textUserProxyHost.text = host;
        textUserProxyPort.text = port;
        proxyUserPassRequired.checked = bRequiresAuth;
        textUserProxyUser.text = user;
        textUserProxyPass.text = pass;
    }
    
    signal done
    signal sigProxyChanges(bool bEnable,
                           bool bUseSystemProxy,
                           string host, int port,
                           bool bRequiresAuth,
                           string user, string pass)

    Container {
        CheckBox {
            id: proxySupport
            text: "Enable proxy support"
        }
        
        CheckBox {
            id: proxySystem
            text: "Use system proxy settings"
            visible: proxySupport.checked
        }
        TextField {
            id: textUserProxyHost
            hintText: "Proxy host"
            visible: proxySupport.checked && !proxySystem.checked
        }
        TextField {
            id: textUserProxyPort
            hintText: "Proxy port"
            visible: proxySupport.checked && !proxySystem.checked
        }

        CheckBox {
            id: proxyUserPassRequired
            text: "Requires user and pass"
            visible: proxySupport.checked && !proxySystem.checked
        }
        TextField {
            id: textUserProxyUser
            hintText: "Proxy username"
            visible: proxyUserPassRequired.visible && proxyUserPassRequired.checked 
        }
        TextField {
            id: textUserProxyPass
            hintText: "Proxy password"
            visible: proxyUserPassRequired.visible && proxyUserPassRequired.checked
        }
        
        Button {
            text: "Submit changes"
            onClicked: {
                container.sigProxyChanges (proxySupport.checked,
                                           proxySystem.checked,
                                           textUserProxyHost.text,
                                           textUserProxyPort.text,
                                           proxyUserPassRequired.checked,
                                           textUserProxyUser.text,
                                           textUserProxyPass.text);
                container.done();
            }
        }
    }
}
