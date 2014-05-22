#include "bfconfig.h"
#include "bf_defs.h"
#include <QProcessEnvironment>
#include <QDebug>

BFConfig::BFConfig(QObject *parent) :
    QObject(parent)
{
}

BFConfig& BFConfig::instance()
{
    static BFConfig conf;
    static bool mNeedInit = true;
    if(mNeedInit)
    {
        conf.init();
        mNeedInit = false;
    }
    return conf;
}

void BFConfig::init()
{
    QStringList environment = QProcessEnvironment::systemEnvironment().toStringList();

    // Вызывается в логе поэтому во избежание зацикливания никаких qDebug()
    qDebug() << environment;
    qDebug() << QProcessEnvironment::systemEnvironment().value("_");

    mInitializedState = 0;
    //TODO //m_DebugLevel = DBG_LVL_ERROR;
    mPidFilePath = QString::fromUtf8("/var/run/bf-service.pid");
    //TODO //m_logLevel = DBG_LVL_ERROR;
    mLogFile = "/dev/null";//mLogFile = "/sintez/sintez/bin/test.log";
    mLogCount = 0;
    mLogMaxFileSize = BARRIER_CONFIG_BF_LOG_MAX_SIZE;

    // Сначала пытаемся загрузить конфигурацию аудита из файла по пути BARRIER_CONFIG_BAU_DEFAULT
    if(QFile::exists(BARRIER_CONFIG_BF_DEFAULT)) {
        mFilePath = BARRIER_CONFIG_BF_DEFAULT;
    }

    // Если неудалось с файлом по умолчанию, загружаем конфигурацию аудита из файла, указанного в переменной окружения по ключу BARRIER_CONFIG_BAU_ENVKEY
    if(mFilePath.isEmpty()) {
        QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
        mFilePath = pe.value(BARRIER_CONFIG_BF_ENVKEY);
    }

    if(mFilePath.isEmpty()) {
        qDebug() << "Can't load configuration: neither " << BARRIER_CONFIG_BF_DEFAULT << "file available nor " << BARRIER_CONFIG_BF_ENVKEY " env variable set";
    } else {
        qDebug() << "Loading configuration from " << mFilePath;
        int res = load(mFilePath);
        if(res < 0) {
            qDebug() << "Can't load configuration from file:" << mFilePath;
        } else {
            mInitializedState = 1;
            qDebug() << "Configuration loaded from from:" << mFilePath;
        }
    }
}

int BFConfig::load(const QString& fPath)
{
    QFile file(fPath);
    if(!file.open(QIODevice::ReadOnly)) {
        return -1;
    }

    QDomDocument document;
    QString errMsg = "";
    int errLine = 0;
    int errColumn = 0;
    if(!document.setContent(&file, &errMsg, &errLine, &errColumn)) {
        file.close();
        qDebug() << errMsg << " : line: " << errLine << ", column: " << errColumn;
        return -2;
    }
    file.close();

    QDomNodeList configs = document.elementsByTagName("config");
    if(configs.size() != 1) {
        qDebug() << "Wrong config file: mismatch `config`";
        return -3;
    }

    QDomNode config = configs.at(0);
    QDomElement configElement = config.toElement();

    QDomNodeList serviceConfigs = configElement.elementsByTagName("service_config");
    if(serviceConfigs.size() == 0) {
        qDebug() << "Wrong config file: mismatch `service_config` section";
        return -4;
    }

    QDomElement serviceConfigElement = serviceConfigs.at(0).toElement();
    if(parseServiceConfig(serviceConfigElement) < 0) {
        qDebug() << "Can't load service configuration.";
        return -5;
    }

    mInitializedState = 1;
    return 0;

}

int BFConfig::parseServiceConfig(QDomElement& serviceConfigElement)
{
    QDomNodeList networks = serviceConfigElement.elementsByTagName("network");
    if(networks.size() > 0) {
        QDomElement networkElement = networks.at(0).toElement();
        //m_ServerAgentClientPort = networkElement.attribute("agent_port", QString::number(DefaultServerPort)).toUShort();
        //m_ServerLiveMonitorPort = networkElement.attribute("live_monitor_port", QString::number(DefaultLiveMonitorPort)).toUShort();
        //m_ServerAgentClientHelperPort = networkElement.attribute("agent_helper_port", QString::number(DefaultServerHelperPort)).toUShort();
    }

    QDomNodeList logs = serviceConfigElement.elementsByTagName("log");
    if(logs.size() > 0) {
        QDomElement logElement = logs.at(0).toElement();
        //TODO //mLogLevel = DebugLevel(logElement.attribute("level", QString::number(DBG_LVL_ERROR)).toInt());
        mLogFile = logElement.attribute("file", "/dev/null");
        mLogCount = logElement.attribute("count", "0").toUShort();
        mLogMaxFileSize = logElement.attribute("max_size", "200000000").toLongLong();
    }

    QDomNodeList processes = serviceConfigElement.elementsByTagName("process");
    if(processes.size() > 0) {
        QDomElement processElement = processes.at(0).toElement();
        mPidFilePath = processElement.attribute("pid", "/tmp/bauserver.pid");
    } else {
        qWarning() << "No process parameters (pid file path,...) defined!";
    }

    return 0;
}
