#ifndef WEBWIDGET_H
#define WEBWIDGET_H

#include "global.h"

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
