#include "DesktopSkypeCallInitiator.h"
#include "Singletons.h"

DesktopSkypeCallInitiator::DesktopSkypeCallInitiator (QObject *parent)
: CalloutInitiator(parent)
, skypeClient (NULL)
{
    // Observer for Skype on desktop Linux and desktop Windows
    skypeClient = Singletons::getRef().getSkypeFactory()
                    .ensureSkypeClient (SKYPE_CLIENT_NAME);
}//DesktopSkypeCallInitiator::DesktopSkypeCallInitiator

void
DesktopSkypeCallInitiator::initiateCall (const QString &strDestination)
{
    do { // Begin cleanup block (not a loop)
        if (NULL == skypeClient) {
            //TODO: We might be able to fix this
            emit log ("Skype not initialized!");
            break;
        }

        QVariantList l;
        l += strDestination;

        bool bOk =
        skypeClient->enqueueWork (SW_InitiateCall, l, this,
            SLOT (onCallInitiated (bool, const QVariantList &)));
        if (!bOk) {
            emit log ("Failed to even begin initiating callout");
            break;
        }
    } while (0); // End cleanup block (not a loop)
}//DesktopSkypeCallInitiator::initiateCall

void
DesktopSkypeCallInitiator::onCallInitiated (bool, const QVariantList &)
{
    emit log ("Callout is successful");
}//DesktopSkypeCallInitiator::onCallInitiated

QString
DesktopSkypeCallInitiator::name ()
{
    return ("skype");
}//DesktopSkypeCallInitiator::
