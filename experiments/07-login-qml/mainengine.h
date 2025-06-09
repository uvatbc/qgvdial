#pragma once

#include <QQmlApplicationEngine>

class MainEngine : public QQmlApplicationEngine {
public:
    MainEngine(QObject *parent = nullptr);

private slots:
    void _slotObjectCreated(QObject *object, const QUrl &url);

private:
    QObject *m_viewObject;
};
