#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>

class Client : public QObject {
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);
    void storeFile(const QString &filePath);
    void retrieveFile(const QString &fileName);

private:
    QTcpSocket *managerSocket;
};

#endif // CLIENT_H