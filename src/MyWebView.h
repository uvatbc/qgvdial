#ifndef __MYWEBVIEW_H__
#define __MYWEBVIEW_H__

#include "global.h"
#include <QtGui>
#include <QtWebKit>

#if !NO_DBGINFO
class MyWebView : public QWebView
{
    Q_OBJECT

public:
    MyWebView (QWidget * parent = 0);

private:
    void keyPressEvent (QKeyEvent *event);

private:
    QString strDbgUrl;
};
#endif

#endif //__MYWEBVIEW_H__
