#ifndef STORE_H
#define STORE_H

#include <QMap>
#include <QString>
#include <QStringList>

class Store: public QObject {
    Q_OBJECT

public:
    explicit Store(){}
    ~Store() {};

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

#endif // STORE_H

