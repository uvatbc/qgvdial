#ifndef DIALERVALIDATOR_H
#define DIALERVALIDATOR_H

#include <QValidator>

class DialerValidator : public QValidator
{
public:
    explicit DialerValidator(QObject *parent = 0);

protected:
    State validate (QString &input, int &pos) const;
};

#endif // DIALERVALIDATOR_H
