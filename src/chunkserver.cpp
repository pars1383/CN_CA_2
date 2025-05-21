#include "chunkserver.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QDebug>
#include "reedsolomon.h"

ChunkServer::ChunkServer(int id, QObject *parent) : QObject(parent), id(id) {
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &ChunkServer::handleConnection);
    server->listen(QHostAddress::Any, 5000 + id);
    QDir().mkpath(QString("./CHUNK-%1").arg(id));
    qDebug() << "ChunkServer" << id << "listening on port" << (5000 + id);

    // DFS traversal order
    chunkServers = {1, 2, 4, 8, 9, 5, 10, 11, 3, 6, 7, 12, 13, 14, 15};
}

void ChunkServer::handleConnection() {
    QTcpSocket *client = server->nextPendingConnection();
    connect(client, &QTcpSocket::readyRead, this, [=]() {
        QJsonObject request = QJsonDocument::fromJson(client->readAll()).object();
        QString fileId = request["file_id"].toString();

        if (request.contains("data")) {
            // Store chunk
            int chunkIndex = request["chunk_index"].toInt();
            QByteArray noisyData = QByteArray::fromBase64(request["data"].toString().toUtf8());
            QByteArray data = decodeData(noisyData);
            if (data.isEmpty()) {
                QJsonObject response;
                response["error"] = "Chunk irrecoverable";
                client->write(QJsonDocument(response).toJson());
                client->disconnectFromHost();
                return;
            }

            QString filePath = QString("./CHUNK-%1/chunk_%2_%3.bin").arg(id).arg(fileId).arg(chunkIndex);
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(data);
                file.close();
            }

            // Send next server address
            QJsonObject response;
            int nextIndex = chunkServers.indexOf(id) + 1;
            if (nextIndex < chunkServers.size()) {
                response["next_server"] = QString("127.0.0.1:%1").arg(5000 + chunkServers[nextIndex]);
            } else {
                response["next_server"] = "";
            }
            client->write(QJsonDocument(response).toJson());
        } else {
            // Retrieve chunk
            QString filePath = QString("./CHUNK-%1/chunk_%2_*.bin").arg(id).arg(fileId);
            QDir dir(QString("./CHUNK-%1").arg(id));
            QStringList files = dir.entryList({QString("chunk_%1_*.bin").arg(fileId)}, QDir::Files);
            if (!files.isEmpty()) {
                QFile file(QString("./CHUNK-%1/%2").arg(id).arg(files[0]));
                if (file.open(QIODevice::ReadOnly)) {
                    QByteArray data = file.readAll();
                    file.close();
                    QByteArray encodedData = encodeData(data);
                    QByteArray noisyData = addNoise(encodedData, 0.01);

                    QJsonObject response;
                    response["data"] = QString(noisyData.toBase64());
                    int nextIndex = chunkServers.indexOf(id) + 1;
                    response["next_server"] = (nextIndex < chunkServers.size()) ?
                        QString("127.0.0.1:%1").arg(5000 + chunkServers[nextIndex]) : "";
                    client->write(QJsonDocument(response).toJson());
                }
            }
        }
        client->disconnectFromHost();
    });
}