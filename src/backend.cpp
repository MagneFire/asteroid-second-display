// SPDX-FileCopyrightText: 2023-2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "backend.h"

#include <QDebug>

namespace SecondDisplay {

std::unique_ptr<Backend> createBackend(const QString &machine)
{
    qInfo() << machine << "has no second display support";
    return std::make_unique<Backend>();
}

}
