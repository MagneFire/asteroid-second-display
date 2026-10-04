// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef HOKI_FACE_H
#define HOKI_FACE_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <cstdint>
#include <optional>

#include "facedescription.h"
#include "sidekicktypes.h"

namespace SecondDisplay {

struct Face
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

    struct Font
    {
        Sidekick::FontInfo info;
        QByteArray png;
    };

    Font font;
    std::optional<Font> minuteFont;
    Bitmap background;
    std::optional<Bitmap> colon;
    Number hours;
    Number minutes;

    QList<std::uint32_t> ids() const;
};

std::optional<FaceDescription> defaultFaceDescription(const QString &directory, int displayWidth);
std::optional<Face> loadFace(const FaceDescription &description, bool twelveHour);

}

#endif
