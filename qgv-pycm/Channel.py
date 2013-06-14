#!/usr/bin/python
 
from time import time
from telepathy.server import ChannelTypeText
from telepathy.server import ChannelTypeStreamedMedia
 
class qgvTextChannel(ChannelTypeText):
    def Send(self, message_type, text):
        # tell the chat client, "yeah, i sent that"
        self.Sent(int(time()), message_type, text)
        # now tell the chat client we got a reply <img src="http://s0.wp.com/wp-includes/images/smilies/icon_smile.gif?m=1129645325g" alt=":-)" class="wp-smiley"> 
        #self.Received(0, int(time()), self._handle.get_id(), message_type, 0, text)
        print "TP asked me to send a text message: %s" % text
 
# make sure the channel shuts down properly
    def Close(self):
        self.remove_from_connection()
        self.Closed()


class qgvStreamedMediaChannel(ChannelTypeStreamedMedia):
    # make sure the channel shuts down properly
    def Close(self):
        self.remove_from_connection()
        self.Closed()
