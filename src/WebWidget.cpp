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
