#include <QCoreApplication>
#include "manager.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Manager manager;
    qDebug() << "Manager started on port 5000";
    return app.exec();
}