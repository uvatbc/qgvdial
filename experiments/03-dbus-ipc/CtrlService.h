#ifndef CTRLSERVICE_H
#define CTRLSERVICE_H

#include "global.h"

class CtrlService : public QObject
{
    Q_OBJECT

public:
    explicit CtrlService(QObject *parent = NULL);

    void requestUserInfo();
    void requestAllQuit();

public Q_SLOTS:
    void ReportUser(const QString &user, const QString &settingsFile,
                    const QString &logsFile, qulonglong clientPid);

Q_SIGNALS:
    void CommandForClient(const QString &command);
};

#endif//CTRLSERVICE_H

