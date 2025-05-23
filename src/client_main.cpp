#include <QCoreApplication>
#include <QThread>
#include "client.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    Client client;
    client.storeFile("data/100KB.txt");
    QThread::sleep(2); // Wait for storage to complete
    client.retrieveFile("100KB.txt", "output.txt");
    return app.exec();
}