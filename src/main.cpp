#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include "client.h"
#include "manager.h"
#include "chunkserver.h"
#include "firewall_punching.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // Start manager
    Manager manager;

    // Start 15 chunk servers
    QList<ChunkServer*> servers;
    for (int i = 1; i <= 15; ++i) {
        servers.append(new ChunkServer(i));
    }

    // Start firewall punching
    FirewallPunching puncher;
    for (int i = 1; i <= 15; ++i) {
        puncher.performHolePunching("127.0.0.1", 5000 + i);
    }

    // Start client and test
    Client client;
    client.storeFile("100KB.txt");
    QThread::sleep(2); // Wait for storage to complete
    client.retrieveFile("100KB.txt", "output.txt");

    return app.exec();
}