// SPDX-FileCopyrightText: 2023 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "backend_catfish.h"

#include <QDebug>

using namespace AsteroidOS::SecondDisplayDaemon;

int CatfishBackend::SynchronizeTime()
{
    qDebug() << "catfish: Syncing time";
    return 1;
}