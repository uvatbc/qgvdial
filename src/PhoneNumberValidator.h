#ifndef PHONENUMBERVALIDATOR_H
#define PHONENUMBERVALIDATOR_H

#include "global.h"

class PhoneNumberValidator : public QValidator
{
public:
    explicit PhoneNumberValidator(QObject *parent = 0);

protected:
    State validate (QString &input, int &pos) const;
};

#endif // PHONENUMBERVALIDATOR_H
