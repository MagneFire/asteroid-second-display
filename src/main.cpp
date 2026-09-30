// SPDX-FileCopyrightText: 2023 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include <stdio.h>
#include <stdlib.h>

#include <QtCore/QCoreApplication>
#include <QtDBus/QtDBus>

#include "backend.h"
#include "dbus.h"
#include "display_adaptor.h"
#include "hands_adaptor.h"

using namespace SecondDisplay;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    // Initialize the backend.
    Backend* backend = Backend::Get();
    Hands* hands = backend->GetHands();

    new DisplayAdaptor(backend);
    new HandsAdaptor(hands);
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerObject(DISPLAY_OBJECT, backend) || !connection.registerObject(HANDS_OBJECT, hands)) {
        qCritical() << "Unable to register objects:" << connection.lastError().message();
        return 1;
    }
    if (!connection.registerService(SERVICE_NAME)) {
        qCritical() << "Unable to register" << SERVICE_NAME << ":" << connection.lastError().message();
        return 1;
    }

    return app.exec();
}
