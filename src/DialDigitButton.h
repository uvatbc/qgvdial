#ifndef DIALDIGITBUTTON_H
#define DIALDIGITBUTTON_H

#include <QPushButton>

class DialDigitButton : public QPushButton
{
    Q_OBJECT

    Q_PROPERTY (QString strChars READ getChars WRITE setChars)

public:
    explicit DialDigitButton (QWidget *parent = 0);
    void setDelete (bool bFlag = true);

    void setChars (const QString &strCh);
    QString getChars ();

signals:
    void charClicked (QChar ch);
    void charDeleted ();

private slots:
    void _i_clicked ();

private:
    bool    bDelete;
    QString strChars;
};

#endif // DIALDIGITBUTTON_H
