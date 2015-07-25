#ifndef STORE_H
#define STORE_H

#include <QMap>
#include <QString>
#include <QStringList>

#define DEF_STORE_VERSION 3

class Store: public QObject {
    Q_OBJECT

public:
    explicit Store(){}
    ~Store() {};
    friend QDataStream &operator << (QDataStream &out, const Store &store);
    friend QDataStream &operator >> (QDataStream &in, Store &store);

public slots:
    void put(QString key, QString value) {
        data.insert(key, value);
        emit changed(*this);
    }

    QString get(QString key) const {
        return data.value(key);
    }

    void remove(QString key) {
        data.remove(key);
        emit changed(*this);
    }

    QStringList get_keys() const{
        return data.keys();
    }

    const QMap<QString,QString> get_data() const {
        return data;
    }
    void set_data(const QMap<QString,QString> dt) {
        data = dt;
        //emit changed(*this);
    }

private:
    QMap<QString,QString> data;

signals:
    void changed(const Store &store);
};

inline QDataStream &operator << (QDataStream &out, const Store &store) {
    out << (qint16)(DEF_STORE_VERSION);
    quint32 elements = store.data.count();
    out << elements;
    for (auto p = store.data.cbegin(); p != store.data.cend(); ++p) {
        QByteArray str;
        str = p.key().toUtf8();
        quint16 klen = str.length();
        out << klen;
        out.writeRawData(str.constData(), klen);

        str = p.value().toUtf8();
        quint32 vlen = str.length();
        out << vlen;
        out.writeRawData(str.constData(), vlen);
    }
    return out;
}

inline QDataStream &operator >> (QDataStream &in, Store &store) {
    store.data.clear();

    qint16 store_version;
    in >> store_version;

    if (store_version == DEF_STORE_VERSION) {
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

            store.put(QString::fromUtf8(key), QString::fromUtf8(val));
        }
    }
    return in;
}

#endif // STORE_H

