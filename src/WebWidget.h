#ifndef WEBWIDGET_H
#define WEBWIDGET_H

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

namespace Ui {
    class WebWidget;
}

class WebWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WebWidget (QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~WebWidget();

private:
    void keyPressEvent (QKeyEvent *event);

private:
    Ui::WebWidget *ui;
};

#endif // WEBWIDGET_H
