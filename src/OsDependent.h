#ifndef __OSDEPENDENT_H__
#define __OSDEPENDENT_H__

#include <QtCore>
#include <QtGui>

class OsDependent : public QObject
{
    Q_OBJECT

public:
    static OsDependent &getRef ();
private:
    OsDependent(QObject *parent = 0);
};

#ifdef QT_NO_SYSTEMTRAYICON
class QSystemTrayIcon : public QWidget
{
    Q_OBJECT
public:
    QSystemTrayIcon(QWidget *parent = 0);
    QSystemTrayIcon(const QIcon &icon, QWidget *parent = 0);
    ~QSystemTrayIcon();

    static bool isSystemTrayAvailable();
    static bool supportsMessages();

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    QString toolTip() const;
    void setToolTip(const QString &tip);

    enum ActivationReason {
        Unknown,
        Context,
        DoubleClick,
        Trigger,
        MiddleClick
    };

Q_SIGNALS:
    void activated(QSystemTrayIcon::ActivationReason reason);
};
#endif

#endif //__OSDEPENDENT_H__
