// SPDX-FileCopyrightText: 2023-2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>
#include <QSettings>

#include "backend.h"
#include "display.h"
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
    Display display(backend.get());
    Hands *hands = backend->GetHands();
    new DisplayAdaptor(&display);
    new HandsAdaptor(hands);

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerObject(DisplayPath, &display) || !bus.registerObject(HandsPath, hands)) {
        qCritical() << "Unable to register objects:" << bus.lastError().message();
        return 1;
    }
    if (!bus.registerService(ServiceName)) {
        qCritical() << "Unable to register" << ServiceName << ":" << bus.lastError().message();
        return 1;
    }

    return app.exec();
}
