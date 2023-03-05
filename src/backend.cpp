#include "backend.h"
#include "backend_catfish.h"

#include <QDebug>
#include <QSettings>

using namespace AsteroidOS::SecondDisplayDaemon;

const char* CONFIG_FILE = "/etc/asteroid/machine.conf";

Backend* Backend::instance;

Backend* Backend::GetBackend()
{
    Backend* backend;
    QSettings m_settings(CONFIG_FILE, QSettings::IniFormat);
    const QString machineCodename = m_settings.value("Identity/MACHINE", "unknown").toString();

    if (machineCodename == "catfish") {
        return new CatfishBackend();
    } else if (machineCodename == "narwhal") {
        // return new NarwhalBackend();
        qDebug() << "asteroid-seconddisplay is not supported on this device!";
        return new Backend();
    } else if (machineCodename == "koi") {
        // return new KoiBackend();
        qDebug() << "asteroid-seconddisplay is not supported on this device!";
        return new Backend();
    }

    qDebug() << "asteroid-seconddisplay is not supported on this device!";
    return new Backend();
}

Backend* Backend::Get()
{
    if (instance == nullptr) {
        instance = GetBackend();
    }
    return instance;
}