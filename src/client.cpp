#include "client.h"
#include <QTcpSocket>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include "reedsolomon.h"

Client::Client(QObject *parent) : QObject(parent) {}

void Client::storeFile(const QString& filePath) {
    qDebug() << "Attempting to store file:" << filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file:" << filePath;
        return;
    }
    qDebug() << "File opened, size:" << file.size();

    // Send request to manager
    QTcpSocket managerSocket;
    managerSocket.connectToHost("127.0.0.1", 5000);
    if (!managerSocket.waitForConnected(10000)) {
        qDebug() << "Cannot connect to manager:" << managerSocket.errorString();
        file.close();
        return;
    }
    qDebug() << "Connected to manager";

    QJsonObject request;
    request["operation"] = "store";
    request["file_name"] = filePath.split("/").last();
    request["file_size"] = file.size();
    request["file_type"] = "text";
    managerSocket.write(QJsonDocument(request).toJson());
    qDebug() << "Sent store request to manager";

    // Receive response
    if (!managerSocket.waitForReadyRead(10000)) {
        qDebug() << "No response from manager:" << managerSocket.errorString();
        file.close();
        return;
    }
    QJsonObject response = QJsonDocument::fromJson(managerSocket.readAll()).object();
    if (response.contains("error")) {
        qDebug() << "Manager error:" << response["error"].toString();
        file.close();
        return;
    }
    QString firstServer = response["first_server"].toString();
    int chunkSize = response["chunk_size"].toInt();
    int numChunks = response["num_chunks"].toInt();
    qDebug() << "Manager response - First server:" << firstServer << "Chunk size:" << chunkSize << "Num chunks:" << numChunks;

    // Split and send chunks
    QString currentServer = firstServer;
    for (int i = 0; i < numChunks; ++i) {
        QByteArray chunk = file.read(chunkSize);
        if (chunk.isEmpty()) {
            qDebug() << "Failed to read chunk" << i;
            file.close();
            return;
        }
        qDebug() << "Read chunk" << i << "size:" << chunk.size();
        QByteArray encodedChunk = encodeData(chunk);
        QByteArray noisyChunk = addNoise(encodedChunk, 0.01);
        qDebug() << "Encoded and added noise to chunk" << i;

        QTcpSocket serverSocket;
        QStringList serverInfo = currentServer.split(":");
        serverSocket.connectToHost(serverInfo[0], serverInfo[1].toInt());
        if (!serverSocket.waitForConnected(10000)) {
            qDebug() << "Cannot connect to chunk server:" << currentServer << serverSocket.errorString();
            file.close();
            return;
        }
        qDebug() << "Connected to chunk server:" << currentServer;

        QJsonObject chunkData;
        chunkData["file_id"] = request["file_name"].toString();
        chunkData["chunk_index"] = i;
        chunkData["data"] = QString(noisyChunk.toBase64());
        serverSocket.write(QJsonDocument(chunkData).toJson());
        qDebug() << "Sent chunk" << i << "to" << currentServer;

        // Get next server
        if (!serverSocket.waitForReadyRead(10000)) {
            qDebug() << "No response from chunk server:" << currentServer << serverSocket.errorString();
            file.close();
            return;
        }
        QJsonObject serverResponse = QJsonDocument::fromJson(serverSocket.readAll()).object();
        if (serverResponse.contains("error")) {
            qDebug() << "Chunk server error:" << serverResponse["error"].toString();
            file.close();
            return;
        }
        currentServer = serverResponse["next_server"].toString();
        qDebug() << "Next server for chunk" << i + 1 << ":" << currentServer;
    }
    file.close();
    qDebug() << "File stored successfully";
}

void Client::retrieveFile(const QString& fileName, const QString& outputPath) {
    qDebug() << "Starting file retrieval for:" << fileName;

    // Send request to manager
    QTcpSocket managerSocket;
    managerSocket.connectToHost("127.0.0.1", 5000);
    if (!managerSocket.waitForConnected(10000)) {
        qDebug() << "Cannot connect to manager for retrieval:" << managerSocket.errorString();
        return;
    }
    qDebug() << "Connected to manager for retrieval";

    QJsonObject request;
    request["operation"] = "retrieve";
    request["file_name"] = fileName;
    managerSocket.write(QJsonDocument(request).toJson());
    qDebug() << "Sent retrieve request for:" << fileName;

    // Receive first server
    if (!managerSocket.waitForReadyRead(10000)) {
        qDebug() << "No response from manager for retrieval:" << managerSocket.errorString();
        return;
    }
    QJsonObject response = QJsonDocument::fromJson(managerSocket.readAll()).object();
    if (response.contains("error")) {
        qDebug() << "Manager error:" << response["error"].toString();
        return;
    }
    QString currentServer = response["first_server"].toString();
    qDebug() << "First server for retrieval:" << currentServer;

    // Retrieve chunks
    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot open output file:" << outputPath;
        return;
    }
    qDebug() << "Opened output file:" << outputPath;

    int chunkIndex = 0;
    while (!currentServer.isEmpty()) {
        QTcpSocket serverSocket;
        QStringList serverInfo = currentServer.split(":");
        serverSocket.connectToHost(serverInfo[0], serverInfo[1].toInt());
        if (!serverSocket.waitForConnected(10000)) {
            qDebug() << "Cannot connect to chunk server:" << currentServer << serverSocket.errorString();
            outputFile.close();
            return;
        }
        qDebug() << "Connected to chunk server:" << currentServer;

        QJsonObject chunkRequest;
        chunkRequest["file_id"] = fileName;
        chunkRequest["chunk_index"] = chunkIndex;
        serverSocket.write(QJsonDocument(chunkRequest).toJson());
        qDebug() << "Sent chunk request for index" << chunkIndex;

        // Receive chunk
        if (!serverSocket.waitForReadyRead(10000)) {
            qDebug() << "No response from chunk server:" << currentServer << serverSocket.errorString();
            outputFile.close();
            return;
        }
        QJsonObject chunkResponse = QJsonDocument::fromJson(serverSocket.readAll()).object();
        if (chunkResponse.contains("error")) {
            qDebug() << "Chunk server error:" << chunkResponse["error"].toString();
            outputFile.close();
            return;
        }
        QByteArray noisyChunk = QByteArray::fromBase64(chunkResponse["data"].toString().toUtf8());
        QByteArray decodedChunk = decodeData(noisyChunk);
        if (decodedChunk.isEmpty()) {
            qDebug() << "Chunk irrecoverable at server:" << currentServer;
            outputFile.close();
            return;
        }
        qDebug() << "Retrieved chunk" << chunkIndex << "size:" << decodedChunk.size();
        outputFile.write(decodedChunk);
        currentServer = chunkResponse["next_server"].toString();
        qDebug() << "Next server:" << currentServer;
        chunkIndex++;
    }
    outputFile.close();
    qDebug() << "File retrieved successfully";
}