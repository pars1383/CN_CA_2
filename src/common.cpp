#include <random>
#include <vector>
// Assume Reed-Solomon library or custom implementation
#include "reedsolomon.h"

QByteArray addNoise(const QByteArray& data, double noiseProbability) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(noiseProbability);

    QByteArray noisyData = data;
    for (int i = 0; i < noisyData.size(); ++i) {
        if (dist(gen)) {
            noisyData[i] ^= (1 << (rand() % 8)); // Flip a random bit
        }
    }
    return noisyData;
}

QByteArray encodeData(const QByteArray& data) {
    // Reed-Solomon encoding (e.g., RS(8192, 8196))
    std::vector<uint8_t> encoded = rs_encode(data.constData(), data.size());
    return QByteArray((char*)encoded.data(), encoded.size());
}

QByteArray decodeData(const QByteArray& data) {
    // Reed-Solomon decoding
    std::vector<uint8_t> decoded = rs_decode(data.constData(), data.size());
    int errorsCorrected = rs_get_errors_corrected();
    qDebug() << "Errors corrected:" << errorsCorrected;
    if (errorsCorrected > rs_max_errors()) {
        qDebug() << "Chunk irrecoverable";
        return QByteArray();
    }
    return QByteArray((char*)decoded.data(), decoded.size());
}