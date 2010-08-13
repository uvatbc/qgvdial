TODO:
-> Call cancel for callout and callback both need to work
-> Call and SMS buttons from contacts page and history page
-> SMS needs to be handled in history: Add children of the SMS as representation of the SMS texts
-> Dialer page buttons need to be mapped to some work
-> Default button on GVSettings page should map to Login button

Changelog:
2010-08-07:
-> Dial callback works.

2010-08-06:
-> Contacts page gets data using Google Contacts Data API
-> Add modify and deletes synchronization from Google servers works.
-> dbus server on telepathy platforms

2010-07-30:
-> Call initiators now need to provide a self number - even if it is currently "undefined"
-> Data API is always used. Webpage dial is no longer used
-> Dial out and dial callback are now two separate features. They are both accessible through settings. Dial callback doesn't work at the moment. Need to fix it.
-> Selecting a registered phone is redundant and is not longer possible.
-> Settings page now merges callouts and callbacks

2010-07-24:
-> Google voice dial out works!!

2010-07-19:
-> Skype integration for Linux and Windows

2010-07-15:
-> Callouts using google voice data API.

2010-07-02:
-> Added cache database table for the GV registered numbers
-> Added init for the table, and accessors and modifiers for its data
-> Registered numbers get queried only if the user is new. Refresh user setting to be done.
-> Registered numbers work starts from the GVSettings page. Not from mainwindow
-> Login now also gets the users GV number
-> This number is sent to the cancel dialog so that Telepathy observer can listen to it.
-> When the observer gets the call, it fires a signal that the cancel dialog recieves and causes the dialog to close itself.
-> Fixed a bug in the setting of callback number: We did not wait for page to complete loading. Now we do.

2010-06-28:
-> Window exit now only hides the window. To really exit, click exit on the menu
-> Single instance tested on Linux, Windows and Maemo.

2010-06-13
-> Maemo now stacks the SMS input window
-> Windows installer
-> History gets cleared on logoff
-> Voicemail mp3 gets cached so that we don't waste network bandwidth
-> Voicemail can now be played in maemo!!!

2010-06-09
-> Send SMS

2010-06-08
-> Maemo 5 specific UI improvement: Dialer buttons now have subscripts.
-> Tabs are on the left so that screen real estate is conserved.
-> Option to place a call or send an SMS from the contacts and history pages.
-> Option on history page to play back voicemail for voicemail items.

2010-06-03
-> Meaningful debug messages that should help me debug problems
-> Check the cancel flag not the fptr for the cancel routine when checking to see if cancel is requested.
-> Fixed an omitted disconnect that was screwing up the call sequence
-> Text message comparison was incorrect
-> SMS messages are now shown in history page

2010-05-27
-> Calling from history page. Select the correct number.
-> Logs go into a log file: <user_dir>/qgvdial.log
-> History event signal: Change from so many strings to one structure please.
-> remove menus when user is not logged in

2010-05-26
-> History page (all)

2010-05-25
-> Changed the contacts list to a Tree View
-> Added menu bar update functions for all tabs.
-> Refresh contacts from menu. Ctrl+R now works as a part of main menu accelerator. No more key hooking.
