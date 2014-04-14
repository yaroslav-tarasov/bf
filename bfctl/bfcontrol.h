#ifndef BFCONTROL_H
#define BFCONTROL_H

#include <QObject>
#include <memory>

struct BFControlPrivate;

class BFControl : public QObject
{
    Q_OBJECT
public:
    explicit BFControl(QObject *parent = 0);
    int  create();
    int  sendMsg(int type,void* msg,size_t size);
    void close();
signals:
    void data(QByteArray ba);
    void done();
public slots:
    void process(QByteArray ba);
    void onData();
private:
    std::shared_ptr<struct BFControlPrivate> d;
};

#endif // BFCONTROL_H
