#include "firewall_punching.h"
#include <QUdpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

FirewallPunching::FirewallPunching(QObject *parent) : QObject(parent)
{
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(0); // Dynamic port
    connect(udpSocket, &QUdpSocket::readyRead, this, &FirewallPunching::handleDatagram);
}

void FirewallPunching::performHolePunching(const QString &peerAddress, int peerPort)
{
    QByteArray data = "PUNCH";
    udpSocket->writeDatagram(data, QHostAddress(peerAddress), peerPort);
    qDebug() << "Sent punch to" << peerAddress << ":" << peerPort;
}

void FirewallPunching::handleDatagram()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        qDebug() << "Received from" << sender.toString() << ":" << senderPort;
        // TCP communication can proceed
    }
}