#!/usr/bin/python
 
import dbus, weakref
from telepathy.server import Connection, ConnectionInterfaceRequests, Handle
from telepathy import HANDLE_TYPE_CONTACT, CONNECTION_STATUS_CONNECTING,\
                      CONNECTION_STATUS_CONNECTED, CONNECTION_STATUS_DISCONNECTED,\
                      CONNECTION_STATUS_REASON_REQUESTED
 
from Constants import PROTOCOL, PROGRAM
from Contacts import qgvContacts
from ChannelManager import qgvChannelManager
 
# many connections per manager -&gt; class for connections
# make a fancy new-type connection with a 'Requests' interface
class qgvConnection(Connection,
                    ConnectionInterfaceRequests,
                    qgvContacts):
 
    def __init__(self, manager, parameters):
        self._manager = weakref.proxy(manager)
        # create a new channel manager and tell it we're it's connection
        self._channel_manager = qgvChannelManager(self)
 
        # assume we have an 'account' name passed to us
        Connection.__init__(self, PROTOCOL, parameters['account'], PROGRAM)
        ConnectionInterfaceRequests.__init__(self)
        qgvContacts.__init__(self)
 
        self._self_handle = Handle(self.get_handle_id(), HANDLE_TYPE_CONTACT,
                                   parameters['account'])
        self._handles[HANDLE_TYPE_CONTACT, self._self_handle.get_id()] =\
                                   self._self_handle
 
    # borrowed from butterfly, required by telepathy's channel init
    def handle(self, handle_type, handle_id):
        self.check_handle(handle_type, handle_id)
        return self._handles[handle_type, handle_id]
 
    def Connect(self):
        self.StatusChanged(CONNECTION_STATUS_CONNECTED,
                           CONNECTION_STATUS_REASON_REQUESTED)
 
    def Disconnect(self):
        self.StatusChanged(CONNECTION_STATUS_DISCONNECTED,
                           CONNECTION_STATUS_REASON_REQUESTED)
        # stop handling all channels
        self._channel_manager.close()
        # stop handling this connection
        self._manager.disconnected(self)

