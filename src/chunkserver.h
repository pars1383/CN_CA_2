#ifndef CHUNKSERVER_H
#define CHUNKSERVER_H
#include <QObject>
#include <QTcpServer>
#include <QVector>

class ChunkServer : public QObject {
    Q_OBJECT
public:
    explicit ChunkServer(int id, QObject *parent = nullptr);
private slots:
    void handleConnection();
private:
    int id;
    QTcpServer *server;
    QVector<int> chunkServers; // DFS order
};
#endif