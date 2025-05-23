#include "manager.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

Manager::Manager(QObject *parent) : QObject(parent) {
    server = new QTcpServer(this);
    if (!server->listen(QHostAddress::Any, 5000)) {
        qDebug() << "Manager failed to listen on port 5000";
        return;
    }
    connect(server, &QTcpServer::newConnection, this, &Manager::handleConnection);
    qDebug() << "Manager listening on port 5000";
}

void Manager::handleConnection() {
    QTcpSocket *socket = server->nextPendingConnection();
    if (!socket->waitForReadyRead(5000)) {
        qDebug() << "Manager: No data from client:" << socket->errorString();
        socket->deleteLater();
        return;
    }

    QJsonObject request = QJsonDocument::fromJson(socket->readAll()).object();
    qDebug() << "Manager received request:" << request;

    QJsonObject response;
    if (request["operation"].toString() == "store") {
        QString fileName = request["file_name"].toString();
        qint64 fileSize = request["file_size"].toDouble();
        int chunkSize = 8192; // 8 KB
        int numChunks = (fileSize + chunkSize - 1) / chunkSize;
        QString firstServer = "127.0.0.1:5001"; // Start at chunk server 1

        QJsonObject fileMetadata;
        fileMetadata["chunk_size"] = chunkSize;
        fileMetadata["num_chunks"] = numChunks;
        fileMetadata["first_server"] = firstServer;
        metadata[fileName] = fileMetadata;

        response["first_server"] = firstServer;
        response["chunk_size"] = chunkSize;
        response["num_chunks"] = numChunks;
        qDebug() << "Manager sending store response for" << fileName << ":" << response;
    } else if (request["operation"].toString() == "retrieve") {
        QString fileName = request["file_name"].toString();
        if (!metadata.contains(fileName)) {
            response["error"] = "File not found";
            qDebug() << "Manager: File not found:" << fileName;
        } else {
            response = metadata[fileName];
            qDebug() << "Manager sending retrieve response for" << fileName << ":" << response;
        }
    } else {
        response["error"] = "Invalid operation";
        qDebug() << "Manager: Invalid operation in request:" << request;
    }

    socket->write(QJsonDocument(response).toJson());
    socket->flush();
    qDebug() << "Manager sent response to client";
    socket->deleteLater();
}