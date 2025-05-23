#include <QCoreApplication>
#include "client.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    Client client;
    client.storeFile("data/100KB.txt");
    client.retrieveFile("100KB.txt");
    return app.exec();
}