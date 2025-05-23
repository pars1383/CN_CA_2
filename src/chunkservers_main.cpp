#include <QCoreApplication>
#include <QDebug>
#include "chunkserver.h"
#include "firewall_punching.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QList<ChunkServer*> servers;
    for (int i = 0; i < 15; ++i) {
        ChunkServer *server = new ChunkServer(i + 1, &app);
        servers.append(server);
        FirewallPunching puncher;
        puncher.performHolePunching("127.0.0.1", 5000 + (i + 1));
        qDebug() << "Sent punch to \"127.0.0.1\" :" << 5000 + (i + 1);
    }
    qDebug() << "Chunk servers started on ports 5001-5015";

    return app.exec();
}