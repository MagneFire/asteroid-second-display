// SPDX-FileCopyrightText: 2023-2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>
#include <QSettings>

#include "backend.h"
#include "dbus.h"
#include "display_adaptor.h"
#include "hands_adaptor.h"

using namespace SecondDisplay;

namespace {

QString machineName()
{
    const QSettings machineConfig("/etc/asteroid/machine.conf", QSettings::IniFormat);
    return machineConfig.value("Identity/MACHINE", "unknown").toString();
}

}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    const auto backend = createBackend(machineName());
    Hands *hands = backend->GetHands();
    new DisplayAdaptor(backend.get());
    new HandsAdaptor(hands);

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerObject(DISPLAY_OBJECT, backend.get()) || !bus.registerObject(HANDS_OBJECT, hands)) {
        qCritical() << "Unable to register objects:" << bus.lastError().message();
        return 1;
    }
    if (!bus.registerService(SERVICE_NAME)) {
        qCritical() << "Unable to register" << SERVICE_NAME << ":" << bus.lastError().message();
        return 1;
    }

    return app.exec();
}
