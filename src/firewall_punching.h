#ifndef FIREWALL_PUNCHING_H
#define FIREWALL_PUNCHING_H
#include <QObject>
#include <QUdpSocket>

class FirewallPunching : public QObject {
    Q_OBJECT
public:
    explicit FirewallPunching(QObject *parent = nullptr);
    void performHolePunching(const QString& peerAddress, int peerPort);
private slots:
    void handleDatagram();
private:
    QUdpSocket *udpSocket;
};
#endif