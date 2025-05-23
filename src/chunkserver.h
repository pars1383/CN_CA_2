#ifndef CHUNKSERVER_H
#define CHUNKSERVER_H
#include <QObject>
#include <QTcpServer>

class ChunkServer : public QObject {
    Q_OBJECT
public:
    explicit ChunkServer(int id, QObject *parent = nullptr);
private slots:
    void handleConnection();
private:
    int serverId;
    QTcpServer *server;
    int getNextServerId(int currentId);
};
#endif