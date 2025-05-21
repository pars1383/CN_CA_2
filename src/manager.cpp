#include "manager.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

Manager::Manager(QObject *parent) : QObject(parent) {
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &Manager::handleConnection);
    server->listen(QHostAddress::Any, 5000);
    qDebug() << "Manager listening on port 5000";

    // DFS traversal order for chunk servers
    chunkServers = {1, 2, 4, 8, 9, 5, 10, 11, 3, 6, 7, 12, 13, 14, 15};
}

void Manager::handleConnection() {
    QTcpSocket *client = server->nextPendingConnection();
    connect(client, &QTcpSocket::readyRead, this, [=]() {
        QJsonObject request = QJsonDocument::fromJson(client->readAll()).object();
        QString operation = request["operation"].toString();

        if (operation == "store") {
            QString fileName = request["file_name"].toString();
            qint64 fileSize = request["file_size"].toInt();
            int numChunks = (fileSize + 8191) / 8192; // Ceiling division
            QString firstServer = QString("127.0.0.1:%1").arg(5000 + chunkServers[0]);

            // Store metadata
            metadata[fileName] = {fileName, fileSize, firstServer};

            QJsonObject response;
            response["first_server"] = firstServer;
            response["chunk_size"] = 8192;
            response["num_chunks"] = numChunks;
            client->write(QJsonDocument(response).toJson());
        } else if (operation == "retrieve") {
            QString fileName = request["file_name"].toString();
            QJsonObject response;
            if (metadata.contains(fileName)) {
                response["first_server"] = metadata[fileName].firstServer;
            } else {
                response["error"] = "File not found";
            }
            client->write(QJsonDocument(response).toJson());
        }
        client->disconnectFromHost();
    });
}