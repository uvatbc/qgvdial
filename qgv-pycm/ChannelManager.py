#!/usr/bin/python
 
import dbus
from telepathy.server import ChannelManager
from telepathy import CHANNEL_INTERFACE, CHANNEL_TYPE_TEXT, CHANNEL_TYPE_STREAMED_MEDIA, HANDLE_TYPE_CONTACT
 
from Channel import qgvTextChannel
 
# Channel Manager with our required channels built in
class qgvChannelManager(ChannelManager):
    def __init__(self, conn):
        self.__text_channel_id = 0
        self.__streamed_media_channel_id = 0

        ChannelManager.__init__(self, conn)
        # ChannelManager magic for handling channels
        self.implement_channel_classes(
            CHANNEL_TYPE_TEXT, self._get_text_channel, [
            # accepting text channels to/from a contact allows empathy to
            # offer 'new conversation'
            ({CHANNEL_INTERFACE + '.ChannelType': CHANNEL_TYPE_TEXT,
              CHANNEL_INTERFACE + '.TargetHandleType': dbus.UInt32(HANDLE_TYPE_CONTACT)},
             [CHANNEL_INTERFACE + '.TargetHandle', CHANNEL_INTERFACE + '.TargetID'])
            ])
        self.implement_channel_classes(
            CHANNEL_TYPE_STREAMED_MEDIA, self._get_streamed_media_channel, [
            # accepting streamed media channels to/from a contact allows empathy to
            # offer 'new call'
            ({CHANNEL_INTERFACE + '.ChannelType': CHANNEL_TYPE_STREAMED_MEDIA,
              CHANNEL_INTERFACE + '.TargetHandleType': dbus.UInt32(HANDLE_TYPE_CONTACT)},
             [CHANNEL_INTERFACE + '.TargetHandle', CHANNEL_INTERFACE + '.TargetID'])
            ])
 
    def _get_text_channel(self, props):
        # make up a name for the channel
        path = "TextChannel%d" % self.__text_channel_id
        self.__text_channel_id += 1
        return qgvTextChannel(self._conn, self, props, object_path=path)
 
    def _get_streamed_media_channel(self, props):
        print str(props)
        # make up a name for the channel
        path = "StreamedMediaChannel%d" % self.__streamed_media_channel_id
        self.__streamed_media_channel_id += 1
        return qgvStreamedMediaChannel(self._conn, self, props, object_path=path)
