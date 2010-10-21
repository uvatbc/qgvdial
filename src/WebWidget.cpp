#include "WebWidget.h"
#include "ui_WebWidget.h"
#include "Singletons.h"

WebWidget::WebWidget(QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f)
, ui(new Ui::WebWidget)
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    ui->setupUi(this);
    webPage.setView (ui->webView);
}//WebWidget::WebWidget

WebWidget::~WebWidget()
{
    delete ui;
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
                            this,
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
        ui->webView->load (url);
    } while (0); // End cleanup block (not a loop)

    if (bIgnore) {
        QWidget::keyPressEvent (event);
    }
}//WebWidget::keyPressEvent
