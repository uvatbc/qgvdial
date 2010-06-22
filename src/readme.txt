TODO:
-> Windows binary needs an icon and that icon needs to be pushed into the installer as well.
-> Same process for package to Maemo.
-> Same process for package to linux.
-> Call and SMS buttons from contacts page and history page
-> Find out NokiaSDK command line parameters for compiling projects directly
-> SMS needs to be handled in history: Add children of the SMS as representation of the SMS texts
-> Skype calling for Windows and Linux
-> Dialer page buttons need to be mapped to some work
-> Default button on GVSettings page should map to Login button

Changelog:
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
