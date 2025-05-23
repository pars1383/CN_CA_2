#ifndef MANAGER_H
#define MANAGER_H
#include <QObject>
#include <QTcpServer>
#include <QJsonObject>
#include <QMap>

class Manager : public QObject {
    Q_OBJECT
public:
    explicit Manager(QObject *parent = nullptr);
private slots:
    void handleConnection();
private:
    QTcpServer *server;
    QMap<QString, QJsonObject> metadata;
};
#endif