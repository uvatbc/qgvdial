#include "DialerWidget.h"
#include "DigitButton.h"
#include <QPushButton>
#include <QRegExp>
#include <QRegExpValidator>
#include "DialerValidator.h"

DialerWidget::DialerWidget(QWidget *parent/* = 0*/, Qt::WindowFlags f/* = 0*/):
QWidget (parent, f),
grid (this),
edNumber (this)
{
    QPushButton *newButton;

    // Select dialing number
    newButton = new QPushButton (this);
    newButton->setText ("Select dialing number");
    grid.addWidget (newButton, 0,0, 1,3);

    // Number to dial
    grid.addWidget (&edNumber, 1,0, 1,3);

    // Select from contacts button
    newButton = new QPushButton (this);
    newButton->setText ("Select contact");
    grid.addWidget (newButton, 2,0, 1,3);

    // Call button
    newButton = new QPushButton (this);
    newButton->setText ("Call");
    QObject::connect (newButton, SIGNAL (clicked ()),
                      this     , SLOT   (btnCallClicked ()));
    grid.addWidget (newButton, 3,0);

    // Backspace
    newButton = new QPushButton (this);
    newButton->setText ("<-");
    QObject::connect (newButton, SIGNAL (clicked ()),
                      this     , SLOT   (backspace()));
    grid.addWidget (newButton, 3,1);

    // +
    newButton = new DigitButton('+', "", this);
    newButton->setText ("+");
    QObject::connect (newButton, SIGNAL (clickedChar (QChar)),
                      this     , SLOT   (digitClicked(QChar)));
    grid.addWidget (newButton, 3,2);

    // Number pad
    QString strKeys = "123456789*0#";
    QStringList arrSubText;
    arrSubText += "";   // 1
    arrSubText += "ABC";// 2
    arrSubText += "DEF";// 3
    arrSubText += "GHI";// 4
    arrSubText += "JKL";// 5
    arrSubText += "MNO";// 6
    arrSubText += "PQRS";// 7
    arrSubText += "TUV";// 8
    arrSubText += "WXYZ";// 9
    arrSubText += "";   // *
    arrSubText += "";   // 0
    arrSubText += "";   // #
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            newButton = new DigitButton(strKeys[(i*3)+j],
                                        arrSubText[(i*3)+j],
                                        this);
            QObject::connect (newButton, SIGNAL (clickedChar (QChar)),
                              this     , SLOT   (digitClicked(QChar)));
            grid.addWidget (newButton, i,j+3);
        }
    }

    QValidator *validator = new DialerValidator(this);
    edNumber.setValidator (validator);

    // Set the grid to the dialer tab
    this->setLayout (&grid);
}//DialerWidget::DialerWidget

DialerWidget::~DialerWidget ()
{
}//DialerWidget::~DialerWidget

void
DialerWidget::digitClicked (QChar chr)
{
    edNumber.insert (QString(chr));
    edNumber.setFocus ();
}//DialerWidget::digitClicked

void
DialerWidget::backspace ()
{
    edNumber.backspace ();
    edNumber.setFocus ();
}//DialerWidget::backspace

void
DialerWidget::btnCallClicked ()
{
    emit call (edNumber.text ());
}//DialerWidget::btnCallClicked

void
DialerWidget::updateMenu (QMenuBar */*menuBar*/)
{
}//DialerWidget::updateMenu
