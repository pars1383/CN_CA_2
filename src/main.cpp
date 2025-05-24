#include <QCoreApplication>
#include <QDebug>
#include "chunkserver.h"
#include "firewall_punching.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Start 15 chunk servers
    QList<ChunkServer *> servers;
    for (int i = 1; i <= 15; ++i)
    {
        servers.append(new ChunkServer(i));
    }

    // Start firewall punching
    FirewallPunching puncher;
    for (int i = 1; i <= 15; ++i)
    {
        puncher.performHolePunching("127.0.0.1", 5000 + i);
    }

    qDebug() << "Chunk servers started on ports 5001-5015";
    return app.exec();
}