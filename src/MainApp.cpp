#include "MainApp.h"

#ifdef Q_WS_WIN32
#pragma comment(lib, "user32.lib")

enum {
// Client is successfully attached and API window handle can be found in wParam parameter
    SKYPECONTROLAPI_ATTACH_SUCCESS              = 0,
// Skype has acknowledged connection request and is waiting for confirmation from the user.
    SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION= 1,
// The client is not yet attached and should wait for SKYPECONTROLAPI_ATTACH_SUCCESS message
// User has explicitly denied access to client
    SKYPECONTROLAPI_ATTACH_REFUSED              = 2,
// API is not available at the moment. For example, this happens when no user is currently logged in.
    SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE        = 3,
// Client should wait for SKYPECONTROLAPI_ATTACH_API_AVAILABLE broadcast before making any further
// connection attempts.
    SKYPECONTROLAPI_ATTACH_API_AVAILABLE        = 0x8001
};

#endif

MainApp::MainApp (int &argc, char **argv)
: QtSingleApplication (argc, argv)
{
#ifdef Q_WS_WIN32
    uidDiscover  = RegisterWindowMessageA ("SkypeControlAPIDiscover");
    uidAttach    = RegisterWindowMessageA ("SkypeControlAPIAttach");
    hSkypeWindow = NULL;
#endif
}//MainApp::MainApp


#ifdef Q_WS_WIN32
bool
MainApp::winEventFilter (MSG *msg, long *result)
{
    bool rv = false;
    do // Begin cleanup block (not a loop)
    {
        if ((WM_COPYDATA  ==        msg->message) &&
            (hSkypeWindow != (HWND) msg->wParam))
        {
            // WM_COPYDATA from an unknown window. Not my problem
            break;
        }

        if ((WM_COPYDATA  != msg->message) &&
            (LOWORD (msg->message) != uidAttach))
        {
            // Not a message we're interested in
            break;
        }

        if (WM_COPYDATA != msg->message)
        {
            // This is an attach message
            switch(msg->lParam)
            {
            case SKYPECONTROLAPI_ATTACH_SUCCESS:
                hSkypeWindow = (HWND) msg->wParam;
                emit skypeAttachStatus (true);
                break;
            case SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION:
                break;
            case SKYPECONTROLAPI_ATTACH_REFUSED:
                emit skypeAttachStatus (false);
                break;
            case SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE:
                emit skypeAttachStatus (false);
                break;
            case SKYPECONTROLAPI_ATTACH_API_AVAILABLE:
                emit skypeAttachStatus (true);
                break;
            }

            *result = 1;
            rv = true;
            break;
        }

        // Copy data message
        COPYDATASTRUCT *cds = (COPYDATASTRUCT *) msg->lParam;
        if (NULL == cds)
        {
            break;
        }

        QString strData = QByteArray ((char *)cds->lpData, cds->cbData);
        emit skypeNotify (strData);

        *result = 1;
        rv = true;
    } while (0); // End cleanup block (not a loop)
    return (rv);
}//MainApp::winEventFilter

UINT
MainApp::getDiscover ()
{
    return (uidDiscover);
}//MainApp::getDiscover

UINT
MainApp::getAttach ()
{
    return (uidAttach);
}//MainApp::getAttach

HWND
MainApp::getSkypeHandle ()
{
    return (hSkypeWindow);
}//MainApp::getSkypeHandle

void
MainApp::clearSkypeHandle ()
{
    hSkypeWindow = NULL;
}//MainApp::clearSkypeHandle

#endif //Q_WS_WIN32
