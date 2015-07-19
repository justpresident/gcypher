#ifndef CYPHER_H
#define CYPHER_H

#include <rijndael.h>
#include <string.h>
#include <store.h>

#include <QMap>
#include <QFileInfo>
#include <QFile>
#include <QDataStream>
#include <QByteArray>

typedef struct cryptstate {
  RIJNDAEL_context ctx;
  UINT8 iv[RIJNDAEL_BLOCKSIZE];
  int mode;
} Crypt__Rijndael;

class Cypher {
public:
    Cypher(const QString file_name, QString key)
        :file_name(file_name)
    {
        while(key.length() < 32) {
            key.append("~");
        }

        UINT8 key_buffer[128];
        size_t key_len = 0;
        memset(key_buffer, 0, 128);
        key = key.left(128);
        for (int i = 0; i < key.length(); ++i) {
            key_buffer[i] = key.at(i).toLatin1();
            key_len++;
        }

        crypt_setup(key_buffer, key_len);
    }
    ~Cypher() {};

    void crypt_setup(const UINT8 *key, size_t key_len) {
        crypt_state.ctx.mode = crypt_state.mode = MODE_CBC;

        memset(crypt_state.iv, 0, RIJNDAEL_BLOCKSIZE);
        rijndael_setup(&crypt_state.ctx, key_len, key);
    }

    void read_data(Store &store) {
        QFileInfo fi(file_name);
        if (fi.exists() && fi.size() > 0) {
            QFile file(file_name);
            file.open(QIODevice::ReadOnly);
            // read data from file to byteArray
            QByteArray byteArray = file.readAll();
            file.close();

            //remove padding length
            char pad_length = byteArray.at(0);
            byteArray.remove(0, 1);

            //decrypt byteArray
            QByteArray decryptedArray(byteArray.length(), 0);
            block_decrypt(&crypt_state.ctx, (UINT8 *)byteArray.data(), byteArray.length(), (UINT8 *)decryptedArray.data(), crypt_state.iv);

            //remove padding
            decryptedArray.remove(decryptedArray.length()-pad_length, pad_length);

            //deserialize data from byteArray
            QDataStream in(decryptedArray);
            in.setVersion(QDataStream::Qt_5_4);
            in >> store;

        }
    }

    void write_data(const Store &store) {
        QByteArray byteArray;
        QDataStream out(&byteArray, QIODevice::ReadWrite);
        out.setVersion(QDataStream::Qt_5_4);

        // serialize data to byteArray
        out << store;
        out.setDevice(0);

        // write padding
        char pad_length = RIJNDAEL_BLOCKSIZE - (byteArray.length() % RIJNDAEL_BLOCKSIZE);
        if (pad_length == RIJNDAEL_BLOCKSIZE)
            pad_length = 0;
        for (qint8 i = 0; i < pad_length; ++i)
            byteArray.append('~');

        // encrypt byteArray
        QByteArray encryptedArray(byteArray.length(), 0);
        block_encrypt(&crypt_state.ctx, (UINT8 *)byteArray.data(), byteArray.length(), (UINT8 *)encryptedArray.data(), crypt_state.iv);

        QFile file(file_name);
        file.open(QIODevice::WriteOnly);
        // write padding length to file
        file.write(&pad_length, 1);

        // write data from byteArray to file
        file.write(encryptedArray);
        file.flush();
        file.close();
    }

    QString get_file_name() const {
        return file_name;
    }

private:
    cryptstate crypt_state;
    QString file_name;
};

#endif // CYPHER_H

