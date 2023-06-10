#include <QFileInfo>
#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include <QtEndian>

#include "cypher.h"

Cypher::Cypher(const QString fileName, const char* key)
    :fileName_(fileName)
{
    UINT8 key_buffer[RIJNDAEL_KEYSIZE];
    memset(key_buffer,0, RIJNDAEL_KEYSIZE);
    memccpy(key_buffer, key, 0, strlen(key));
    for(quint8 i = strlen(key); i < RIJNDAEL_KEYSIZE; ++i) {
        key_buffer[i] = '~';
    }

    crypt_setup(key_buffer, RIJNDAEL_KEYSIZE);
}

void Cypher::crypt_setup(const UINT8 *key, size_t key_len) {
    cryptState_.ctx.mode = cryptState_.mode = MODE_CBC;

    memset(cryptState_.iv, 0, RIJNDAEL_BLOCKSIZE);
    rijndael_setup(&cryptState_.ctx, key_len, key);
}

quint8 Cypher::read_data(Store &store) {
    QFileInfo fi(fileName_);
    if (!fi.exists() || fi.size() == 0)
        return ERR_CYPHER_EMPTY_FILE;

    QFile file(fileName_);
    file.open(QIODevice::ReadOnly);
    // read data from file to byteArray

    quint16 cypher_version;
    file.read((char *)&cypher_version, sizeof(cypher_version));
    cypher_version = qToBigEndian(cypher_version);

    if (cypher_version != DEF_CYPHER_VERSION)
        return ERR_CYPHER_BAD_FORMAT;

    QByteArray byteArray = file.readAll();
    file.close();

    //remove padding length
    quint8 pad_length = byteArray.at(0);
    byteArray.remove(0, 1);

    //decrypt byteArray
    QByteArray decryptedArray(byteArray.length(), 0);
    block_decrypt(&cryptState_.ctx, (UINT8 *)byteArray.data(), byteArray.length(), (UINT8 *)decryptedArray.data(), cryptState_.iv);

    //remove padding
    decryptedArray.remove(decryptedArray.length()-pad_length, pad_length);

    //deserialize data from byteArray
    QDataStream in(decryptedArray);
    in.setVersion(QDataStream::Qt_5_4);
    in >> store;

    return 0;
}

void Cypher::write_data(const Store &store) {
    QByteArray byteArray;
    QDataStream out(&byteArray, QIODevice::ReadWrite);
    out.setVersion(QDataStream::Qt_5_4);

    // serialize data to byteArray
    out << store;
    out.setDevice(0);

    // write padding
    quint8 pad_length = RIJNDAEL_BLOCKSIZE - (byteArray.length() % RIJNDAEL_BLOCKSIZE);
    if (pad_length == RIJNDAEL_BLOCKSIZE)
        pad_length = 0;
    for (qint8 i = 0; i < pad_length; ++i)
        byteArray.append('~');

    // encrypt byteArray
    QByteArray encryptedArray(byteArray.length(), 0);
    block_encrypt(&cryptState_.ctx, (UINT8 *)byteArray.data(), byteArray.length(), (UINT8 *)encryptedArray.data(), cryptState_.iv);

    // open file
    QFile file(fileName_);
    file.open(QIODevice::WriteOnly);
    out.setDevice(&file);

    //write version
    out << (quint16)(DEF_CYPHER_VERSION);

    // write padding length to file
    out << pad_length;
    out.setDevice(0);

    // write data from byteArray to file
    file.write(encryptedArray);
    file.flush();
    file.close();
}

QString Cypher::get_file_name() const {
    return fileName_;
}
