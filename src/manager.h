#ifndef MANAGER_H
#define MANAGER_H
#include <QObject>
#include <QTcpServer>
#include <QMap>

struct FileMetadata {
    QString fileName;
    qint64 fileSize;
    QString firstServer;
};

class Manager : public QObject {
    Q_OBJECT
public:
    explicit Manager(QObject *parent = nullptr);
private slots:
    void handleConnection();
private:
    QTcpServer *server;
    QMap<QString, FileMetadata> metadata;
    QVector<int> chunkServers; // DFS order
};
#endif