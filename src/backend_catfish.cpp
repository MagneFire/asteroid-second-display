#include "backend_catfish.h"

#include <QDebug>

using namespace AsteroidOS::SecondDisplayDaemon;

int CatfishBackend::SynchronizeTime()
{
    qDebug() << "catfish: Syncing time";
    return 1;
}