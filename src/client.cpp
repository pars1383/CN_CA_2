#include "client.h"
#include <QTcpSocket>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include "reedsolomon.h"

Client::Client(QObject *parent) : QObject(parent) {}

void Client::storeFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file:" << filePath;
        return;
    }

    // Send request to manager
    QTcpSocket managerSocket;
    managerSocket.connectToHost("127.0.0.1", 5000); // Localhost
    if (!managerSocket.waitForConnected()) {
        qDebug() << "Cannot connect to manager";
        return;
    }

    QJsonObject request;
    request["operation"] = "store";
    request["file_name"] = filePath.split("/").last();
    request["file_size"] = file.size();
    request["file_type"] = "text";
    managerSocket.write(QJsonDocument(request).toJson());

    // Receive response
    managerSocket.waitForReadyRead();
    QJsonObject response = QJsonDocument::fromJson(managerSocket.readAll()).object();
    QString firstServer = response["first_server"].toString();
    int chunkSize = response["chunk_size"].toInt();
    int numChunks = response["num_chunks"].toInt();

    // Split and send chunks
    QString currentServer = firstServer;
    for (int i = 0; i < numChunks; ++i) {
        QByteArray chunk = file.read(chunkSize);
        QByteArray encodedChunk = encodeData(chunk); // Reed-Solomon encoding
        QByteArray noisyChunk = addNoise(encodedChunk, 0.01); // 1% noise

        QTcpSocket serverSocket;
        QStringList serverInfo = currentServer.split(":");
        serverSocket.connectToHost(serverInfo[0], serverInfo[1].toInt());
        if (!serverSocket.waitForConnected()) {
            qDebug() << "Cannot connect to chunk server:" << currentServer;
            return;
        }

        QJsonObject chunkData;
        chunkData["file_id"] = request["file_name"].toString();
        chunkData["chunk_index"] = i;
        chunkData["data"] = QString(noisyChunk.toBase64());
        serverSocket.write(QJsonDocument(chunkData).toJson());

        // Get next server
        serverSocket.waitForReadyRead();
        QJsonObject serverResponse = QJsonDocument::fromJson(serverSocket.readAll()).object();
        currentServer = serverResponse["next_server"].toString();
    }
    file.close();
    qDebug() << "File stored successfully";
}

void Client::retrieveFile(const QString& fileName, const QString& outputPath) {
    // Send request to manager
    QTcpSocket managerSocket;
    managerSocket.connectToHost("127.0.0.1", 5000); // Localhost
    if (!managerSocket.waitForConnected()) {
        qDebug() << "Cannot connect to manager";
        return;
    }

    QJsonObject request;
    request["operation"] = "retrieve";
    request["file_name"] = fileName;
    managerSocket.write(QJsonDocument(request).toJson());

    // Receive first server
    managerSocket.waitForReadyRead();
    QJsonObject response = QJsonDocument::fromJson(managerSocket.readAll()).object();
    QString currentServer = response["first_server"].toString();

    // Retrieve chunks
    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot open output file:" << outputPath;
        return;
    }

    while (!currentServer.isEmpty()) {
        QTcpSocket serverSocket;
        QStringList serverInfo = currentServer.split(":");
        serverSocket.connectToHost(serverInfo[0], serverInfo[1].toInt());
        if (!serverSocket.waitForConnected()) {
            qDebug() << "Cannot connect to chunk server:" << currentServer;
            return;
        }

        QJsonObject chunkRequest;
        chunkRequest["file_id"] = fileName;
        serverSocket.write(QJsonDocument(chunkRequest).toJson());

        // Receive chunk
        serverSocket.waitForReadyRead();
        QJsonObject chunkResponse = QJsonDocument::fromJson(serverSocket.readAll()).object();
        QByteArray noisyChunk = QByteArray::fromBase64(chunkResponse["data"].toString().toUtf8());
        QByteArray decodedChunk = decodeData(noisyChunk); // Reed-Solomon decoding
        if (decodedChunk.isEmpty()) {
            qDebug() << "Chunk irrecoverable at server:" << currentServer;
            outputFile.close();
            return;
        }
        currentServer = chunkResponse["next_server"].toString();
        outputFile.write(decodedChunk);
    }
    outputFile.close();
    qDebug() << "File retrieved successfully";
}