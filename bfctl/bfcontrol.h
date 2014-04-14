#ifndef BFCONTROL_H
#define BFCONTROL_H

#include <QObject>

class BFControl : public QObject
{
    Q_OBJECT
public:
    explicit BFControl(QObject *parent = 0);
    
signals:
    
public slots:
    void onDataArrival(QByteArray ba);
    void onData();

};

#endif // BFCONTROL_H
