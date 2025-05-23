#include "chunkserver.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QFile>
#include <QDebug>

ChunkServer::ChunkServer(int id, QObject *parent) : QObject(parent), serverId(id) {
    server = new QTcpServer(this);
    if (!server->listen(QHostAddress::Any, 5000 + id)) {
        qDebug() << "ChunkServer" << id << "failed to listen on port" << (5000 + id);
        return;
    }
    connect(server, &QTcpServer::newConnection, this, &ChunkServer::handleConnection);
    qDebug() << "ChunkServer" << id << "listening on port" << (5000 + id);
}

void ChunkServer::handleConnection() {
    QTcpSocket *socket = server->nextPendingConnection();
    if (!socket->waitForReadyRead(5000)) {
        qDebug() << "ChunkServer" << serverId << "no data from client:" << socket->errorString();
        socket->deleteLater();
        return;
    }

    QJsonObject request = QJsonDocument::fromJson(socket->readAll()).object();
    qDebug() << "ChunkServer" << serverId << "received request:" << request;

    if (request.contains("file_id")) {
        QString fileId = request["file_id"].toString();
        int chunkIndex = request["chunk_index"].toInt(-1);

        if (chunkIndex >= 0) {
            // Store chunk
            QString dirPath = QString("CHUNK-%1").arg(serverId);
            QDir().mkpath(dirPath);
            QString filePath = QString("%1/chunk_%2_%3.bin").arg(dirPath, fileId).arg(chunkIndex);
            QFile file(filePath);
            if (!file.open(QIODevice::WriteOnly)) {
                qDebug() << "ChunkServer" << serverId << "cannot open file:" << filePath;
                QJsonObject response;
                response["error"] = "Cannot open chunk file";
                socket->write(QJsonDocument(response).toJson());
                socket->deleteLater();
                return;
            }
            QByteArray data = QByteArray::fromBase64(request["data"].toString().toUtf8());
            file.write(data);
            file.close();
            qDebug() << "ChunkServer" << serverId << "stored chunk:" << filePath << "size:" << data.size();

            // Send response with next server
            QJsonObject response;
            int nextId = (chunkIndex + 1 < 15) ? getNextServerId(serverId) : 0;
            response["next_server"] = nextId ? QString("127.0.0.1:%1").arg(5000 + nextId) : "";
            socket->write(QJsonDocument(response).toJson());
            qDebug() << "ChunkServer" << serverId << "sent response, next server:" << response["next_server"].toString();
        } else {
            // Retrieve chunk
            QString filePath = QString("CHUNK-%1/chunk_%2_%3.bin").arg(serverId).arg(fileId).arg(request["chunk_index"].toInt());
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly)) {
                qDebug() << "ChunkServer" << serverId << "cannot open chunk:" << filePath;
                QJsonObject response;
                response["error"] = "Chunk not found";
                socket->write(QJsonDocument(response).toJson());
                socket->deleteLater();
                return;
            }
            QByteArray data = file.readAll();
            file.close();
            qDebug() << "ChunkServer" << serverId << "read chunk:" << filePath << "size:" << data.size();

            QJsonObject response;
            response["data"] = QString(data.toBase64());
            int nextId = getNextServerId(serverId);
            response["next_server"] = nextId ? QString("127.0.0.1:%1").arg(5000 + nextId) : "";
            socket->write(QJsonDocument(response).toJson());
            qDebug() << "ChunkServer" << serverId << "sent chunk, next server:" << response["next_server"].toString();
        }
    }
    socket->deleteLater();
}

int ChunkServer::getNextServerId(int currentId) {
    // DFS order: 1, 2, 4, 8, 9, 5, 10, 11, 3, 6, 7, 12, 13, 14, 15
    int order[] = {1, 2, 4, 8, 9, 5, 10, 11, 3, 6, 7, 12, 13, 14, 15};
    for (int i = 0; i < 14; ++i) {
        if (order[i] == currentId) {
            return order[i + 1];
        }
    }
    return 0;
}