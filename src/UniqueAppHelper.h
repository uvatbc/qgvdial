#ifndef UNIQUEAPPHELPER_H
#define UNIQUEAPPHELPER_H

#include <QtCore>

class UniqueAppHelper : public QObject
{
    Q_OBJECT

public:
    static UniqueAppHelper &getRef ();
    bool setUnique ();
    void signalOriginal ();
    bool isWakeSignaled ();

signals:
    void log(const QString &strText, int level = 10);

private:
    UniqueAppHelper();
    ~UniqueAppHelper();
    bool writeChar (char ch);
    bool readChar (char &ch);
    bool writeOwner ();
    bool readOwner (quint64 &pid);

    quint64 currentPid();
    bool isPidValid (quint64 pid);

private:
    QFile fUnique;
    bool  bOwner;
};

#endif // UNIQUEAPPHELPER_H
