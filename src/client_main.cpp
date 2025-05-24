#include <QCoreApplication>
#include "client.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Client client;
    client.storeFile("data/fun_computer_250kb_A.txt");
    client.retrieveFile("fun_computer_250kb_A.txt");
    return app.exec();
}