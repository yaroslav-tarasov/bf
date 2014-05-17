#ifndef ERRORRECEIVER_H
#define ERRORRECEIVER_H

#include <QObject>

class ErrorReciever : public QObject
{
    Q_OBJECT

public:
    explicit ErrorReciever(QObject *parent) :
        QObject(parent),mErr(0)
    {

    }

    quint16 getError(){return mErr;}
public slots:
    void setError(quint16 e){mErr = e;}


private:
    quint16 mErr;
};
#endif // ERRORRECEIVER_H
