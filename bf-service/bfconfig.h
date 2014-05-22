#ifndef BFCONFIG_H
#define BFCONFIG_H

#include <QString>
#include <QDomDocument>
#include <QFile>
#include <QStringList>

class BFConfig : public QObject
{
    Q_OBJECT
public:
    static BFConfig&        instance();          // singleton accessor
    inline QString          rulesCachePath() const { return QString("/sintez/sintez/bin/.rules"); }
    inline quint8           logCount()       const { return mLogCount; }
    inline qint64           logMaxFileSize() const { return mLogMaxFileSize; }
    inline const QString&   logFile()        const { return mLogFile; }
    int                     load(const QString& fPath);
    void                    init();

signals:
    
public slots:
private:
    Q_DISABLE_COPY(BFConfig)
    explicit                 BFConfig(QObject *parent = 0);
    int                      parseServiceConfig(QDomElement& serviceConfigElement);
//// Параметры системы
    qint8    mInitializedState; // Состояние  (0 - неинициализировано, 1 - инициализировано)
    QString  mFilePath;         // Файл конфигурации
    QString  mLogFile;          // Файл протоколирования. По умолчанию /dev/null
    quint8   mLogCount;         // Кол-во предыдущих файлов протоколирования. По умолчанию, 0. Т.е. только один - текущий.
    qint64   mLogMaxFileSize;   // Размер файла протокола, по достижении которого файл ротируется. Если mLogCount==0, то текущий файл закрывается, обнуляется и запись начинается заново.
    QString  mPidFilePath;      // Путь к pid файлу
////

};

#endif // BFCONFIG_H
