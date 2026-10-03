// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef TIMEPIECEFACE_H
#define TIMEPIECEFACE_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <cstdint>
#include <optional>

#include "sidekicktypes.h"

namespace SecondDisplay {

struct TimepieceFace
{
    struct Bitmap
    {
        Sidekick::DrawableInfo drawable;
        QByteArray png;
    };

    struct Number
    {
        Sidekick::DrawableInfo drawable;
        Sidekick::NumberInfo number;
    };

    Sidekick::FontInfo font;
    QByteArray fontPng;
    Bitmap background;
    Bitmap colon;
    Number hours;
    Number minutes;

    QList<std::uint32_t> ids() const;
};

std::optional<TimepieceFace> loadTimepieceFace(const QString &directory, bool twelveHour, int displayWidth);

}

#endif
