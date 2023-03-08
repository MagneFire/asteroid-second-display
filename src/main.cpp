#include <stdio.h>
#include <stdlib.h>

#include <QtCore/QCoreApplication>
#include <QtDBus/QtDBus>

#include "backend.h"
#include "dbus.h"
#include "display_adaptor.h"
#include "hands_adaptor.h"

using namespace AsteroidOS::SecondDisplayDaemon;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    // Initialize the backend.
    Backend* backend = Backend::Get();
    Hands* hands = backend->GetHands();

    new DisplayAdaptor(backend);
    new HandsAdaptor(hands);
    QDBusConnection connection = QDBusConnection::systemBus();
    connection.registerObject(DISPLAY_OBJECT, backend);
    connection.registerObject(HANDS_OBJECT, hands);
    connection.registerService(SERVICE_NAME);

    return app.exec();
}
