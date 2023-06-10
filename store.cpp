#include "store.h"



void Store::put(const QString key, const QString val, const quint32 ts, bool emit_change) {
    TValuePair pair = {val, ts};
    if (data_.contains(key)) {
        data_[key].append(pair);
    } else {
        data_.insert(key, {pair});
    }
    if (emit_change)
        emit changed(*this);
}

QString Store::get(QString key) const {
    return data_.value(key).at(0).first; // FIXME !! sort by timestamp
}

void Store::remove(QString key) {
    data_.remove(key);
    emit changed(*this);
}

QStringList Store::get_keys() const{
    return data_.keys();
}

const TKeyValue Store::get_data() const {
    return data_;
}

QDataStream &operator << (QDataStream &out, const Store &store) {
    out << (qint16)(DEF_STORE_VERSION);
    quint32 elements = store.data_.count();
    out << elements;
    for (auto p = store.data_.cbegin(); p != store.data_.cend(); ++p) {
        QByteArray str; // buffer for both key and value

        const TValuesArray valarr = p.value();
        for (const auto& val_pair: valarr) {
            str = p.key().toUtf8();
            quint16 klen = str.length();
            out << klen;
            out.writeRawData(str.constData(), klen);

            str = val_pair.first.toUtf8();
            quint32 vlen = str.length();
            out << vlen;
            out.writeRawData(str.constData(), vlen);

            if (DEF_STORE_VERSION == STORE_VER_4) {
                out << quint32(val_pair.second);
            }
        }
    }
    return out;
}

QDataStream &operator >> (QDataStream &in, Store &store) {
    store.data_.clear();

    qint16 store_version;
    in >> store_version;

    if (store_version == STORE_VER_3 || store_version == STORE_VER_4) {
        quint32 elements;
        in >> elements;
        for (quint32 i = 0; i < elements; ++i) {
            quint16 klen;
            in >> klen;
            char key[klen + 1];
            in.readRawData(key, klen);
            key[klen] = 0;

            quint32 vlen;
            in >> vlen;
            char val[vlen + 1];
            in.readRawData(val, vlen);
            val[vlen] = 0;

            quint32 ts = 0;
            if (store_version == STORE_VER_4) {
                in >> ts;
            }

            store.put(QString::fromUtf8(key), QString::fromUtf8(val), ts, 0);
        }
    }
    return in;
}
