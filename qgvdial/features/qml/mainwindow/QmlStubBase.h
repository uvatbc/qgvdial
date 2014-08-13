#ifndef QMLSTUBOBJECT_H
#define QMLSTUBOBJECT_H

#include <QObject>

class QmlStubBase : public QObject
{
    Q_OBJECT
public:
    explicit QmlStubBase(QObject *parent = 0);
    Q_INVOKABLE QObject *findChild(QString name);
};

#endif // QMLSTUBOBJECT_H
