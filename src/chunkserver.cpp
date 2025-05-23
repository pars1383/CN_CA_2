#include "chunkserver.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

// CN_CA_2: Added chunk validation to prevent empty storage

ChunkServer::ChunkServer(int id, QObject *parent) : QObject(parent), serverId(id) {
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &ChunkServer::handleNewConnection);
    QString dirName = QString("CHUNK-%1").arg(id);
    QDir().mkdir(dirName);
}

bool ChunkServer::listen(quint16 port) {
    return server->listen(QHostAddress::LocalHost, port);
}

void ChunkServer::handleNewConnection() {
    QTcpSocket *clientSocket = server->nextPendingConnection();
    connect(clientSocket, &QTcpSocket::readyRead, this, [=]() { handleRequest(clientSocket); });
    connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);
}

void ChunkServer::handleRequest(QTcpSocket *clientSocket) {
    QByteArray requestData;
    while (clientSocket->bytesAvailable() || clientSocket->waitForReadyRead(10000)) {
        requestData += clientSocket->readAll();
    }

    QJsonDocument doc = QJsonDocument::fromJson(requestData);
    if (doc.isNull()) {
        qDebug() << "ChunkServer" << serverId << "received invalid JSON";
        QJsonObject response;
        response["status"] = "error";
        response["message"] = "Invalid JSON";
        QJsonDocument responseDoc(response);
        clientSocket->write(responseDoc.toJson());
        clientSocket->flush();
        return;
    }

    QJsonObject request = doc.object();
    QString fileId = request["file_id"].toString();
    int chunkIndex = request["chunk_index"].toInt();
    QString chunkPath = QString("CHUNK-%1/chunk_%2_%3.bin").arg(serverId).arg(fileId).arg(chunkIndex);

    if (request.contains("data")) {
        // Store chunk
        QString data = request["data"].toString();
        QByteArray decodedData = QByteArray::fromBase64(data.toUtf8());
        if (decodedData.isEmpty()) {
            qDebug() << "ChunkServer" << serverId << "received empty chunk for:" << chunkPath;
            QJsonObject response;
            response["status"] = "error";
            response["message"] = "Empty chunk data";
            QJsonDocument responseDoc(response);
            clientSocket->write(responseDoc.toJson());
            clientSocket->flush();
            return;
        }

        QFile file(chunkPath);
        if (file.open(QIODevice::WriteOnly)) {
            qint64 bytesWritten = file.write(decodedData);
            file.close();
            qDebug() << "ChunkServer" << serverId << "stored chunk:" << chunkPath << "size:" << bytesWritten;
        } else {
            qDebug() << "ChunkServer" << serverId << "failed to open file:" << chunkPath;
            QJsonObject response;
            response["status"] = "error";
            response["message"] = "Failed to open file";
            QJsonDocument responseDoc(response);
            clientSocket->write(responseDoc.toJson());
            clientSocket->flush();
            return;
        }

        // Send response
        QJsonObject response;
        response["status"] = "success";
        response["next_server"] = QString("127.0.0.1:%1").arg(5001 + (serverId % 15));
        QJsonDocument responseDoc(response);
        clientSocket->write(responseDoc.toJson());
        clientSocket->flush();
        qDebug() << "ChunkServer" << serverId << "sent response, next server:" << response["next_server"].toString();
    } else {
        // Retrieve chunk
        QFile file(chunkPath);
        QByteArray data;
        if (file.open(QIODevice::ReadOnly)) {
            data = file.readAll();
            file.close();
            qDebug() << "ChunkServer" << serverId << "read chunk:" << chunkPath << "size:" << data.size();
        } else {
            qDebug() << "ChunkServer" << serverId << "failed to read chunk:" << chunkPath;
            QJsonObject response;
            response["status"] = "error";
            response["message"] = "Failed to read chunk";
            QJsonDocument responseDoc(response);
            clientSocket->write(responseDoc.toJson());
            clientSocket->flush();
            return;
        }

        QJsonObject response;
        response["status"] = "success";
        response["data"] = QString(data.toBase64());
        response["next_server"] = QString("127.0.0.1:%1").arg(5001 + (serverId % 15));
        QJsonDocument responseDoc(response);
        clientSocket->write(responseDoc.toJson());
        clientSocket->flush();
        qDebug() << "ChunkServer" << serverId << "sent response, next server:" << response["next_server"].toString();
    }
}