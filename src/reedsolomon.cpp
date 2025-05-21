#include "reedsolomon.h"
#include <random>
#include <QDebug>

// Simplified Reed-Solomon (for demo purposes, assumes RS(255, 251) with 4 parity bytes)
QByteArray addNoise(const QByteArray& data, double noiseProbability) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(noiseProbability);

    QByteArray noisyData = data;
    bool corrupted = false;
    for (int i = 0; i < noisyData.size(); ++i) {
        if (dist(gen) || (i == 0 && !corrupted)) { // Ensure at least one bit flip
            noisyData[i] ^= (1 << (rand() % 8));
            corrupted = true;
        }
    }
    return noisyData;
}

QByteArray encodeData(const QByteArray& data) {
    // Simplified: Add 4 parity bytes (dummy implementation)
    QByteArray encoded = data;
    for (int i = 0; i < 4; ++i) {
        encoded.append(static_cast<char>(0)); // Explicitly cast to char
    }
    return encoded;
}

QByteArray decodeData(const QByteArray& data) {
    // Simplified: Check first 4 bytes for errors (dummy implementation)
    int errors = 0;
    for (int i = 0; i < data.size(); ++i) {
        if (data[i] != 0 && i < data.size() - 4) { // Assume errors in non-parity
            errors++;
        }
    }
    qDebug() << "Errors detected:" << errors;
    if (errors > 2) { // Max 2 errors correctable
        qDebug() << "Chunk irrecoverable";
        return QByteArray();
    }
    return data.left(data.size() - 4); // Remove parity
}