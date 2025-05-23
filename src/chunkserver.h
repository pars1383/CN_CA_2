#ifndef CHUNKSERVER_H
#define CHUNKSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class ChunkServer : public QObject {
    Q_OBJECT
public:
    explicit ChunkServer(int id, QObject *parent = nullptr);
    bool listen(quint16 port);

private slots:
    void handleNewConnection();
    void handleRequest(QTcpSocket *clientSocket);

private:
    QTcpServer *server;
    int serverId;
};

#endif // CHUNKSERVER_H