#pragma once

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
    void put(const QString key, const QString val, const quint32 ts = 0, bool emit_change = 1);

    void remove(QString key);
public:
    QString get(QString key) const;

    QStringList get_keys() const;

    const TKeyValue get_data() const;


private:
    TKeyValue data_;

signals:
    void changed(const Store &store);
};

QDataStream &operator << (QDataStream &out, const Store &store);
QDataStream &operator >> (QDataStream &in, Store &store);
