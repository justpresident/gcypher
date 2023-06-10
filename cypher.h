#ifndef CYPHER_H
#define CYPHER_H

#include <string.h>

#include "rijndael.h"
#include "store.h"

typedef struct cryptstate {
  RIJNDAEL_context ctx;
  UINT8 iv[RIJNDAEL_BLOCKSIZE];
  int mode;
} Crypt__Rijndael;

#define DEF_CYPHER_VERSION 2

#define ERR_CYPHER_BAD_FORMAT 1
#define ERR_CYPHER_EMPTY_FILE 2

class Cypher {
public:
    Cypher(const QString fileName, const char* key);
    ~Cypher() = default;

    void crypt_setup(const UINT8 *key, size_t key_len);

    quint8 read_data(Store &store);

    void write_data(const Store &store);

    QString get_file_name() const;

private:
    cryptstate cryptState_;
    QString fileName_;
};

#endif // CYPHER_H

