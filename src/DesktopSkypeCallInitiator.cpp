#include "DesktopSkypeCallInitiator.h"
#include "Singletons.h"
#include "SkypeClient.h"

DesktopSkypeCallInitiator::DesktopSkypeCallInitiator (QObject *parent)
: CalloutInitiator(parent)
, skypeClient (NULL)
{
}//DesktopSkypeCallInitiator::DesktopSkypeCallInitiator

void
DesktopSkypeCallInitiator::initiateCall (const QString &strDestination)
{
    bool bOk;
    do { // Begin cleanup block (not a loop)
        if (NULL == skypeClient) {
            skypeClient = Singletons::getRef().getSkypeFactory()
                            .ensureSkypeClient (SKYPE_CLIENT_NAME);
            if (NULL == skypeClient) {
                qWarning ("Failed to create skype Client!");
                break;
            }
        }

        // Save it for onSkypeConnected
        strNumber = strDestination;

        QVariantList l;
        if (!skypeClient->isConnected ()) {
            bOk =
            skypeClient->enqueueWork (SW_Connect, l, this,
                SLOT (onSkypeConnected (bool, const QVariantList &)));
            if (!bOk) {
                qWarning ("Could not connect skype!!!");
            }
            break;
        }

        onSkypeConnected (true, l);
    } while (0); // End cleanup block (not a loop)
}//DesktopSkypeCallInitiator::initiateCall

void
DesktopSkypeCallInitiator::onSkypeConnected (bool bSuccess, const QVariantList&)
{
    bool bOk;
    do { // Begin cleanup block (not a loop)
        if (!bSuccess) {
            qWarning ("Failed to connect to skype");
            break;
        }

        QVariantList l;
        l += strNumber;
        bOk =
        skypeClient->enqueueWork (SW_InitiateCall, l, this,
            SLOT (onCallInitiated (bool, const QVariantList &)));
        if (!bOk) {
            qWarning ("Failed to even begin initiating callout");
            break;
        }
    } while (0); // End cleanup block (not a loop)
}//DesktopSkypeCallInitiator::onSkypeConnected

void
DesktopSkypeCallInitiator::onCallInitiated (bool, const QVariantList &)
{
    qDebug ("Callout is successful");
}//DesktopSkypeCallInitiator::onCallInitiated

QString
DesktopSkypeCallInitiator::name ()
{
    return ("skype");
}//DesktopSkypeCallInitiator::name

QString
DesktopSkypeCallInitiator::selfNumber ()
{
    return ("undefined");
}//DesktopSkypeCallInitiator::selfNumber
