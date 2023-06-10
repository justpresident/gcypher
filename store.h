#ifndef STORE_H
#define STORE_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QDataStream>

#define STORE_VER_3 3
#define STORE_VER_4 4
#define DEF_STORE_VERSION STORE_VER_4

typedef QPair<QString, quint32> TValuePair;
typedef QVector<TValuePair> TValuesArray;
typedef QMap<QString,TValuesArray> TKeyValue;

class Store: public QObject {
    Q_OBJECT

public:
    explicit Store(){}
    ~Store() {};
    friend QDataStream & operator << (QDataStream &out, const Store &store);
    friend QDataStream & operator >> (QDataStream &in, Store &store);

public slots:
    void put(const QString key, const QString val, const quint32 ts = 0, bool emit_change = 1) {
        TValuePair pair = {val, ts};
        if (data.contains(key)) {
            data[key].append(pair);
        } else {
            data.insert(key, {pair});
        }
        if (emit_change)
            emit changed(*this);
    }

    QString get(QString key) const {
        return data.value(key).at(0).first; // FIXME !! sort by timestamp
    }

    void remove(QString key) {
        data.remove(key);
        emit changed(*this);
    }

    QStringList get_keys() const{
        return data.keys();
    }

    const TKeyValue get_data() const {
        return data;
    }
    void set_data(const TKeyValue dt) {
        data = dt;
        //emit changed(*this);
    }

private:
    TKeyValue data;

signals:
    void changed(const Store &store);
};

inline QDataStream &operator << (QDataStream &out, const Store &store) {
    out << (qint16)(DEF_STORE_VERSION);
    quint32 elements = store.data.count();
    out << elements;
    for (auto p = store.data.cbegin(); p != store.data.cend(); ++p) {
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
                out << val_pair.second;
            }
        }
    }
    return out;
}

inline QDataStream &operator >> (QDataStream &in, Store &store) {
    store.data.clear();

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

#endif // STORE_H

