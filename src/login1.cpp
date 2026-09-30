// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "login1.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>

namespace SecondDisplay {

bool requestPowerOff()
{
    QDBusMessage powerOff = QDBusMessage::createMethodCall(
        "org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "PowerOff");
    powerOff << false;
    const QDBusMessage reply = QDBusConnection::systemBus().call(powerOff);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "login1 PowerOff failed:" << reply.errorMessage();
        return false;
    }
    return true;
}

}
