#include "client.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QCoreApplication>
#include "reedsolomon.h"

// CN_CA_2: Enhanced error handling and retrieval logic

Client::Client(QObject *parent) : QObject(parent)
{
    managerSocket = new QTcpSocket(this);
}

void Client::storeFile(const QString &filePath)
{
    qDebug() << "Attempting to store file:" << filePath;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open file:" << filePath;
        return;
    }
    qint64 fileSize = file.size();
    qDebug() << "File opened, size:" << fileSize;

    // Connect to manager
    managerSocket->connectToHost("127.0.0.1", 5000);
    if (!managerSocket->waitForConnected(10000))
    {
        qDebug() << "Failed to connect to manager";
        return;
    }
    qDebug() << "Connected to manager";

    // Send store request
    QJsonObject request;
    request["operation"] = "store";
    request["file_name"] = filePath.split("/").last();
    request["file_size"] = fileSize;
    request["file_type"] = "text";
    QJsonDocument doc(request);
    managerSocket->write(doc.toJson());
    managerSocket->flush();
    qDebug() << "Sent store request to manager";

    // Read manager response
    if (!managerSocket->waitForReadyRead(15000))
    {
        qDebug() << "No response from manager";
        file.close();
        return;
    }
    QJsonDocument responseDoc = QJsonDocument::fromJson(managerSocket->readAll());
    QJsonObject response = responseDoc.object();
    QString firstServer = response["first_server"].toString();
    int chunkSize = response["chunk_size"].toInt();
    int numChunks = response["num_chunks"].toInt();
    qDebug() << "Manager response - First server:" << firstServer << "Chunk size:" << chunkSize << "Num chunks:" << numChunks;

    // Store chunks
    QString currentServer = firstServer;
    bool storageSuccess = true;
    for (int i = 0; i < numChunks; ++i)
    {
        QByteArray chunk = file.read(chunkSize);
        qDebug() << "Read chunk" << i << "size:" << chunk.size();
        QByteArray encodedChunk = encodeData(chunk);
        if (encodedChunk.isEmpty())
        {
            qDebug() << "Encoding failed for chunk" << i;
            storageSuccess = false;
            break;
        }
        encodedChunk = addNoise(encodedChunk, 0.01); // Apply noise after encoding
        qDebug() << "Encoded and added noise to chunk" << i << "size:" << encodedChunk.size();

        // Retry connection up to 3 times
        bool chunkStored = false;
        for (int retry = 0; retry < 3; ++retry)
        {
            QTcpSocket chunkSocket;
            QString host = currentServer.split(":").first();
            int port = currentServer.split(":").last().toInt();
            chunkSocket.connectToHost(host, port);
            if (!chunkSocket.waitForConnected(10000))
            {
                qDebug() << "Failed to connect to chunk server:" << currentServer << "Retry" << retry + 1;
                continue;
            }
            qDebug() << "Connected to chunk server:" << currentServer;

            QJsonObject chunkRequest;
            chunkRequest["file_id"] = request["file_name"].toString();
            chunkRequest["chunk_index"] = i;
            chunkRequest["data"] = QString(encodedChunk.toBase64());
            QJsonDocument chunkDoc(chunkRequest);
            chunkSocket.write(chunkDoc.toJson());
            chunkSocket.flush();
            qDebug() << "Sent chunk" << i << "to" << currentServer;

            if (chunkSocket.waitForReadyRead(15000))
            {
                QJsonDocument chunkResponseDoc = QJsonDocument::fromJson(chunkSocket.readAll());
                QJsonObject chunkResponse = chunkResponseDoc.object();
                if (chunkResponse["status"].toString() == "success")
                {
                    currentServer = chunkResponse["next_server"].toString();
                    qDebug() << "Received response for chunk" << i << "Next server:" << currentServer;
                    chunkStored = true;
                    break;
                }
                else
                {
                    qDebug() << "Invalid response for chunk" << i << ":" << chunkResponse["message"].toString();
                }
            }
            else
            {
                qDebug() << "No response from chunk server:" << currentServer << chunkSocket.errorString();
            }
        }
        if (!chunkStored)
        {
            qDebug() << "Failed to store chunk" << i << "after retries";
            storageSuccess = false;
            break;
        }
    }
    file.close();
    managerSocket->disconnectFromHost();

    if (storageSuccess)
    {
        qDebug() << "File stored successfully";
        retrieveFile(request["file_name"].toString());
    }
    else
    {
        qDebug() << "File storage failed, skipping retrieval";
    }
}

void Client::retrieveFile(const QString &fileName)
{
    qDebug() << "Starting file retrieval for:" << fileName;

    // Ensure socket is not connected
    if (managerSocket->state() != QAbstractSocket::UnconnectedState)
    {
        managerSocket->disconnectFromHost();
        managerSocket->waitForDisconnected(1000);
    }

    // Connect to manager
    managerSocket->connectToHost("127.0.0.1", 5000);
    if (!managerSocket->waitForConnected(10000))
    {
        qDebug() << "Failed to connect to manager for retrieval";
        return;
    }
    qDebug() << "Connected to manager for retrieval";

    // Send retrieve request
    QJsonObject request;
    request["operation"] = "retrieve";
    request["file_name"] = fileName;
    QJsonDocument doc(request);
    managerSocket->write(doc.toJson());
    managerSocket->flush();
    qDebug() << "Sent retrieve request for:" << fileName;

    // Read manager response
    if (!managerSocket->waitForReadyRead(15000))
    {
        qDebug() << "No response from manager for retrieval";
        return;
    }
    QJsonDocument responseDoc = QJsonDocument::fromJson(managerSocket->readAll());
    QJsonObject response = responseDoc.object();
    QString firstServer = response["first_server"].toString();
    int chunkSize = response["chunk_size"].toInt();
    int numChunks = response["num_chunks"].toInt();
    qDebug() << "First server for retrieval:" << firstServer << "Chunk size:" << chunkSize << "Num chunks:" << numChunks;

    // Retrieve chunks
    QFile outputFile("output.txt");
    if (!outputFile.open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to open output file: output.txt";
        return;
    }
    qDebug() << "Opened output file: output.txt";

    QString currentServer = firstServer;
    for (int i = 0; i < numChunks; ++i)
    {
        // Retry connection up to 3 times
        bool chunkRetrieved = false;
        for (int retry = 0; retry < 3; ++retry)
        {
            QTcpSocket chunkSocket;
            QString host = currentServer.split(":").first();
            int port = currentServer.split(":").last().toInt();
            chunkSocket.connectToHost(host, port);
            if (!chunkSocket.waitForConnected(10000))
            {
                qDebug() << "Failed to connect to chunk server:" << currentServer << "Retry" << retry + 1;
                continue;
            }
            qDebug() << "Connected to chunk server:" << currentServer;

            QJsonObject chunkRequest;
            chunkRequest["file_id"] = fileName;
            chunkRequest["chunk_index"] = i;
            QJsonDocument chunkDoc(chunkRequest);
            chunkSocket.write(chunkDoc.toJson());
            chunkSocket.flush();
            qDebug() << "Sent chunk request for index" << i;

            if (chunkSocket.waitForReadyRead(15000))
            {
                QJsonDocument chunkResponseDoc = QJsonDocument::fromJson(chunkSocket.readAll());
                QJsonObject chunkResponse = chunkResponseDoc.object();
                if (chunkResponse["status"].toString() == "success")
                {
                    QByteArray decodedChunk = QByteArray::fromBase64(chunkResponse["data"].toString().toUtf8());
                    QByteArray decodedData = decodeData(decodedChunk);
                    if (decodedData.isEmpty())
                    {
                        qDebug() << "Decoding failed for chunk" << i;
                    }
                    else
                    {
                        if (decodedData.size() > chunkSize)
                        {
                            qDebug() << "Warning: Decoded chunk" << i << "size" << decodedData.size() << "exceeds expected" << chunkSize;
                            decodedData = decodedData.left(chunkSize);
                        }
                        outputFile.write(decodedData);
                    }
                    currentServer = chunkResponse["next_server"].toString();
                    qDebug() << "Received chunk" << i << "size:" << decodedData.size() << "Next server:" << currentServer;
                    chunkRetrieved = true;
                    break;
                }
                else
                {
                    qDebug() << "Invalid response for chunk" << i << ":" << chunkResponse["message"].toString();
                }
            }
            else
            {
                qDebug() << "No response from chunk server:" << currentServer << chunkSocket.errorString();
            }
        }
        if (!chunkRetrieved)
        {
            qDebug() << "Failed to retrieve chunk" << i << "after retries";
            break;
        }
    }
    outputFile.close();
    managerSocket->disconnectFromHost();
    qDebug() << "File retrieved successfully";
}