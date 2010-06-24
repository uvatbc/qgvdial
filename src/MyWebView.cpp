#include "MyWebView.h"

#if !NO_DBGINFO

MyWebView::MyWebView (QWidget * parent /*= 0*/)
: QWebView (parent)
{
}//MyWebView::MyWebView

void
MyWebView::keyPressEvent (QKeyEvent *event)
{
    if ((Qt::Key_N == event->key ()) &&
        (Qt::ControlModifier & event->modifiers ()))
    {
        bool bOk = false;
        strDbgUrl = QInputDialog::getText (this,
                                           "Sideways hack",
                                           "Load URL",
                                           QLineEdit::Normal,
                                           strDbgUrl,
                                           &bOk);
        if (bOk && !strDbgUrl.isEmpty ())
        {
            this->load (QUrl (strDbgUrl));
        }
    }
    else
    {
        QWebView::keyPressEvent (event);
    }
}//MyWebView::keyPressEvent

#endif //!NO_DBGINFO
