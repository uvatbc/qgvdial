#include "WebWidget.h"
#include "Singletons.h"

WebWidget::WebWidget(QDeclarativeItem *parent)
: QDeclarativeItem(parent)
, wv (new QWebView)
, proxy (new QGraphicsProxyWidget(this))
{
    proxy->setWidget(wv);

    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    webPage.setView (wv);
}//WebWidget::WebWidget

WebWidget::~WebWidget()
{
}//WebWidget::~WebWidget

void
WebWidget::keyPressEvent (QKeyEvent *event)
{
    bool bIgnore = true;
    do // Begin cleanup block (not a loop)
    {
        if (Qt::Key_N != event->key ())
        {
            break;
        }

        if (Qt::ControlModifier != event->modifiers ())
        {
            break;
        }
        bIgnore = false;

        // Ask for a new page input
        bool ok = false;
        QString strUrl = QInputDialog::getText(
                            NULL,
                            tr("Enter new URL"),
                            tr("URL:"),
                            QLineEdit::Normal,
                            tr ("http://"),
                            &ok);
        if (!ok)
        {
            qDebug ("User canceled URL input");
            break;
        }

        QUrl url = QUrl::fromUserInput (strUrl);
        wv->load (url);
    } while (0); // End cleanup block (not a loop)

    if (bIgnore) {
        QDeclarativeItem::keyPressEvent (event);
    }
}//WebWidget::keyPressEvent
