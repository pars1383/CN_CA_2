#include "reedsolomon.h"
#include <random>
#include <QDebug>
#include "schifra_galois_field.hpp"
#include "schifra_reed_solomon_encoder.hpp"
#include "schifra_reed_solomon_decoder.hpp"
#include "schifra_reed_solomon_block.hpp"
#include "schifra_sequential_root_generator_polynomial_creator.hpp"

// RS(255, 251) with 4 parity symbols (corrects up to 2 errors)
#define CODE_LENGTH 255
#define FEC_LENGTH 4
#define DATA_LENGTH (CODE_LENGTH - FEC_LENGTH)

QByteArray addNoise(const QByteArray& data, double noiseProbability) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(noiseProbability);

    QByteArray noisyData = data;
    int errors = 0;
    // Apply noise per block (255 bytes), max 2 errors per block
    for (qsizetype i = 0; i < data.size() && errors < 2; i += CODE_LENGTH) {
        for (qsizetype j = i; j < i + CODE_LENGTH && j < data.size() && errors < 2; ++j) {
            if (dist(gen)) {
                noisyData[j] ^= (1 << (rand() % 8));
                errors++;
            }
        }
    }
    qDebug() << "Added" << errors << "errors to data";
    return noisyData;
}

QByteArray encodeData(const QByteArray& data) {
    // Initialize Galois field for GF(2^8) with primitive polynomial
    const unsigned int primitive_poly[] = {1, 0, 1, 1, 1, 0, 0, 0, 1}; // x^8 + x^4 + x^3 + x^2 + 1
    schifra::galois::field field(8, primitive_poly);
    
    // Create generator polynomial
    schifra::galois::field_polynomial generator(field, FEC_LENGTH);
    if (!schifra::make_sequential_root_generator_polynomial(field, FEC_LENGTH, 0, generator)) {
        qDebug() << "Failed to create generator polynomial";
        return QByteArray();
    }
    
    // Validate polynomial
    if (generator.deg() != FEC_LENGTH) {
        qDebug() << "Invalid generator polynomial degree:" << generator.deg() << "expected:" << FEC_LENGTH;
        return QByteArray();
    }
    
    // Debug polynomial
    qDebug() << "Generator polynomial degree:" << generator.deg();
    for (std::size_t i = 0; i <= generator.deg(); ++i) {
        qDebug() << "Term" << i << ":" << generator[i].index();
    }

    schifra::reed_solomon::encoder<CODE_LENGTH, FEC_LENGTH> encoder(field, generator);
    QByteArray encoded;

    // Process data in blocks of up to DATA_LENGTH (251) bytes
    for (qsizetype i = 0; i < data.size(); i += DATA_LENGTH) {
        schifra::reed_solomon::block<CODE_LENGTH, FEC_LENGTH> block;
        qsizetype blockSize = std::min<qsizetype>(DATA_LENGTH, data.size() - i);
        for (qsizetype j = 0; j < blockSize; ++j) {
            block[j] = static_cast<unsigned char>(data[i + j]);
        }
        for (qsizetype j = blockSize; j < DATA_LENGTH; ++j) {
            block[j] = 0; // Pad with zeros
        }

        if (!encoder.encode(block)) {
            qDebug() << "Failed to encode block at offset" << i;
            return QByteArray();
        }

        // Append the entire codeword (data + parity)
        for (qsizetype j = 0; j < CODE_LENGTH; ++j) {
            encoded.append(static_cast<char>(block[j]));
        }
    }
    qDebug() << "Encoded data: input size" << data.size() << "output size" << encoded.size();
    return encoded;
}

QByteArray decodeData(const QByteArray& data) {
    // Initialize Galois field for GF(2^8) with primitive polynomial
    const unsigned int primitive_poly[] = {1, 0, 1, 1, 1, 0, 0, 0, 1}; // x^8 + x^4 + x^3 + x^2 + 1
    schifra::galois::field field(8, primitive_poly);
    
    // Create generator polynomial (same as encoder)
    schifra::galois::field_polynomial generator(field, FEC_LENGTH);
    if (!schifra::make_sequential_root_generator_polynomial(field, FEC_LENGTH, 0, generator)) {
        qDebug() << "Failed to create generator polynomial for decoder";
        return QByteArray();
    }
    
    schifra::reed_solomon::decoder<CODE_LENGTH, FEC_LENGTH> decoder(field, 0);
    QByteArray decoded;

    // Process data in blocks of CODE_LENGTH (255) bytes
    for (qsizetype i = 0; i < data.size(); i += CODE_LENGTH) {
        if (i + CODE_LENGTH > data.size()) {
            qDebug() << "Incomplete block at offset" << i << "size" << data.size() - i;
            break;
        }

        schifra::reed_solomon::block<CODE_LENGTH, FEC_LENGTH> block;
        for (qsizetype j = 0; j < CODE_LENGTH; ++j) {
            block[j] = static_cast<unsigned char>(data[i + j]);
        }

        qDebug() << "Decoding block at offset" << i << "input size" << CODE_LENGTH;
        if (!decoder.decode(block)) {
            qDebug() << "Failed to decode block at offset" << i;
            continue; // Skip invalid block
        }

        // Append the data portion (excluding parity)
        for (qsizetype j = 0; j < DATA_LENGTH; ++j) {
            decoded.append(static_cast<char>(block[j]));
        }
    }
    // Trim padding zeros
    while (decoded.size() > 0 && decoded.endsWith('\0')) {
        decoded.chop(1);
    }
    qDebug() << "Decoded data: input size" << data.size() << "output size" << decoded.size();
    return decoded;
}