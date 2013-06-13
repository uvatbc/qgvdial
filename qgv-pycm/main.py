#!/usr/bin/python
 
# IMPORTANT! makes asynchronous dbus things work
from dbus.mainloop.glib import DBusGMainLoop
DBusGMainLoop(set_as_default=True)
 
# get the mainloop before creating the ConnectionManager
# so that dbus (telepathy) can use it
from gobject import MainLoop
ml = MainLoop()

from ConnectionManager import qgvConnectionManager
 
# get telepathy to start listening for our dbus stuff via the mainloop
qgvConnectionManager()
# and ... Go!
ml.run()

