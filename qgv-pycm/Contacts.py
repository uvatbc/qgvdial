#!/usr/bin/python
 
import dbus
from telepathy.server import ConnectionInterfaceContacts
from telepathy import CONNECTION, CONNECTION_INTERFACE_CONTACTS, HANDLE_TYPE_CONTACT
 
# Contacts interface with our minimal requirements implemented
class qgvContacts(ConnectionInterfaceContacts):
    def __init__(self):
        ConnectionInterfaceContacts.__init__(self)
        self._implement_property_get(CONNECTION_INTERFACE_CONTACTS,
            {'ContactAttributeInterfaces' :
            lambda:  dbus.Array([CONNECTION], signature='s')})
 
    # Overwrite the dbus attribute to get the sender argument
    @dbus.service.method(CONNECTION_INTERFACE_CONTACTS, in_signature='auasb',
                         out_signature='a{ua{sv}}', sender_keyword='sender')
    def GetContactAttributes(self, handles, interfaces, hold, sender):
        # this is required to allow the channel to close down correctly
        if hold: self.HoldHandles(HANDLE_TYPE_CONTACT, handles, sender)
 
        ret = dbus.Dictionary(signature='ua{sv}')
        for handle in handles:
            ret[handle] = dbus.Dictionary(signature='sv')
            ret[handle][CONNECTION + "/contact-id"] =\
                self.InspectHandles(HANDLE_TYPE_CONTACT, [handle])[0]
        return ret

