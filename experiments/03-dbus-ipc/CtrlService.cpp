#include "CtrlService.h"

CtrlService::CtrlService(QObject *parent)
: QObject(parent)
{
}//CtrlService::CtrlService

void
CtrlService::ReportUser(const QString &user, const QString &settingsFile,
                        const QString &logsFile, qulonglong clientPid)
{
    QString msg = QString("Reporting user %1, settings %2, logs %3, pid %4")
                    .arg(user, settingsFile, logsFile).arg(clientPid);
    Q_DEBUG(msg);
}//CtrlService::ReportUser

void
CtrlService::requestUserInfo()
{
    emit CommandForClient("getUser");
}//CtrlService::requestUserInfo

void
CtrlService::requestAllQuit()
{
    emit CommandForClient("quitAll");
}//CtrlService::requestAllQuit

