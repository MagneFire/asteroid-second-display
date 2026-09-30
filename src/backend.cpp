// SPDX-FileCopyrightText: 2023 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "backend.h"

#include <QDebug>
#include <QSettings>

using namespace AsteroidOS::SecondDisplayDaemon;

const char* CONFIG_FILE = "/etc/asteroid/machine.conf";

Backend* Backend::instance;

Backend* Backend::GetBackend()
{
    QSettings m_settings(CONFIG_FILE, QSettings::IniFormat);
    const QString machineCodename = m_settings.value("Identity/MACHINE", "unknown").toString();

    qInfo() << machineCodename << "has no second display support";
    return new Backend();
}

Backend* Backend::Get()
{
    if (instance == nullptr) {
        instance = GetBackend();
    }
    return instance;
}
