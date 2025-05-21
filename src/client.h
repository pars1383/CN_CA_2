#ifndef CLIENT_H
#define CLIENT_H
#include <QObject>

class Client : public QObject {
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);
    void storeFile(const QString& filePath);
    void retrieveFile(const QString& fileName, const QString& outputPath);
};
#endif