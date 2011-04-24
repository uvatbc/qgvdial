#ifndef MQPUBLISHER_H
#define MQPUBLISHER_H

#include "global.h"
#include <mosquitto.h>

class MqPublisher : public QObject
{
    Q_OBJECT

public:
    MqPublisher(const QString &name,
                const QString &strServer, int port,
                const QString &strTopic,
                QObject *parent = 0);

public slots:
    void publish (const QByteArray &byPayload);
    void on_mqConnected (int result);
    void on_mqDisonnected ();
    void on_mqPublish (uint16_t mid);

private:
    QString m_strName;
    QString m_strServer;
    int m_port;
    QString m_strTopic;
    QByteArray m_byPayload;

    bool bConnected;
    struct mosquitto *mosq;
};

#endif // MQPUBLISHER_H
