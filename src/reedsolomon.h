#ifndef REEDSOLOMON_H
#define REEDSOLOMON_H
#include <QByteArray>

QByteArray addNoise(const QByteArray& data, double noiseProbability);
QByteArray encodeData(const QByteArray& data);
QByteArray decodeData(const QByteArray& data);
#endif