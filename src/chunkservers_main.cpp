#include <QCoreApplication>
#include <QDebug>
#include "chunkserver.h"
#include "firewall_punching.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QList<ChunkServer *> servers;
    for (int i = 0; i < 15; ++i)
    {
        ChunkServer *server = new ChunkServer(i + 1, &app);
        if (server->listen(5001 + i))
        {
            servers.append(server);
            FirewallPunching puncher;
            puncher.performHolePunching("127.0.0.1", 5001 + i);
            qDebug() << "Sent punch to \"127.0.0.1\" :" << 5001 + i;
        }
        else
        {
            qDebug() << "Failed to start ChunkServer" << i + 1 << "on port" << 5001 + i;
        }
    }
    qDebug() << "Chunk servers started on ports 5001-5015";

    return app.exec();
}