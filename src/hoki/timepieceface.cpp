// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "timepieceface.h"

#include <QDebug>
#include <QFile>
#include <QtEndian>
#include <cmath>

namespace SecondDisplay {

using namespace Sidekick;

namespace {

constexpr int DesignSize = 416;
constexpr int Glyphs = 10;
constexpr int Gap = 10;
constexpr int DigitsTop = 148;
constexpr float MsPerMinute = 60 * 1000.0f;
constexpr float MsPerHour = 60 * MsPerMinute;

constexpr std::int32_t BackgroundId = 1;
constexpr std::int32_t FontId = 2;
constexpr std::int32_t HoursId = 3;
constexpr std::int32_t MinutesId = 4;
constexpr std::int32_t ColonId = 5;

bool pngSize(const QByteArray &png, std::int32_t *width, std::int32_t *height)
{
    if (png.size() < 24 || !png.startsWith("\x89PNG"))
        return false;
    *width = qFromBigEndian<quint32>(png.constData() + 16);
    *height = qFromBigEndian<quint32>(png.constData() + 20);
    return true;
}

std::optional<QByteArray> readFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to read" << path;
        return std::nullopt;
    }
    return file.readAll();
}

DrawableInfo drawable(std::int32_t id, DrawableType type, int x, int y, int width, int height, int zOrder)
{
    DrawableInfo info;
    info.id = id;
    info.type = type;
    info.offsetX = x;
    info.offsetY = y;
    info.width = width;
    info.height = height;
    info.zOrder = zOrder;
    info.display = true;
    info.displayInTwm = true;
    return info;
}

Time timeFromOffset(long long timeOffsetMs)
{
    constexpr long long MsPerDay = 24 * 60 * 60 * 1000LL;
    Time time;
    time.daysSinceLocalEpoch = static_cast<std::int32_t>(timeOffsetMs / MsPerDay);
    time.msSinceMidnight = static_cast<std::int32_t>(timeOffsetMs % MsPerDay);
    if (time.msSinceMidnight < 0) {
        time.msSinceMidnight += MsPerDay;
        time.daysSinceLocalEpoch--;
    }
    return time;
}

NumberInfo number(int lowest, int highest, float msPerIncrement, long long timeOffsetMs, int minDigitsShown)
{
    NumberInfo info;
    info.startTime = timeFromOffset(-timeOffsetMs);
    info.startNumber = lowest;
    info.minNumber = lowest;
    info.maxNumber = highest;
    info.increment = 1;
    info.msPerIncrement = msPerIncrement;
    info.fontId = FontId;
    info.leadingZeroes = std::max(0, minDigitsShown - 1);
    const int needed = highest <= 0 ? 1 : static_cast<int>(std::log(highest) / std::log(Glyphs)) + 1;
    info.digits = std::max(minDigitsShown, needed);
    info.digit = -1;
    info.transitionMs = 0;
    return info;
}

}

QList<std::uint32_t> TimepieceFace::ids() const
{
    return {static_cast<std::uint32_t>(font.id), static_cast<std::uint32_t>(background.drawable.id),
            static_cast<std::uint32_t>(colon.drawable.id), static_cast<std::uint32_t>(hours.drawable.id),
            static_cast<std::uint32_t>(minutes.drawable.id)};
}

std::optional<TimepieceFace> loadTimepieceFace(const QString &directory, bool twelveHour, int displayWidth)
{
    const auto background = readFile(directory + "/background.png");
    const auto digits = readFile(directory + "/digits.png");
    const auto colon = readFile(directory + "/colon.png");
    if (!background || !digits || !colon)
        return std::nullopt;

    TimepieceFace face;
    std::int32_t width = 0;
    std::int32_t height = 0;
    if (!pngSize(*digits, &width, &height) || height % Glyphs != 0) {
        qWarning() << "digits.png is not a strip of" << Glyphs << "glyphs";
        return std::nullopt;
    }
    face.font.id = FontId;
    face.font.width = width;
    face.font.height = height / Glyphs;
    face.font.nGlyphs = Glyphs;
    face.fontPng = *digits;

    const float scale = static_cast<float>(displayWidth) / DesignSize;
    const int glyphWidth = face.font.width;
    const int top = static_cast<int>(DigitsTop * scale);
    const int centre = displayWidth / 2;

    if (!pngSize(*background, &width, &height))
        return std::nullopt;
    face.background.drawable = drawable(BackgroundId, DrawableType::Generic, 0, 0, width, height, 0);
    face.background.png = *background;

    if (!pngSize(*colon, &width, &height))
        return std::nullopt;
    face.colon.drawable = drawable(ColonId, DrawableType::Generic, centre - width / 2, top, width, height, 1);
    face.colon.png = *colon;

    const int gap = static_cast<int>(Gap * scale) + width / 2;
    face.hours.drawable = drawable(HoursId, DrawableType::Number, centre - gap - 2 * glyphWidth, top, 100, 50, 2);
    face.minutes.drawable = drawable(MinutesId, DrawableType::Number, centre + gap, top, 100, 50, 3);
    if (twelveHour)
        face.hours.number = number(1, 12, MsPerHour, -static_cast<long long>(MsPerHour), 1);
    else
        face.hours.number = number(0, 23, MsPerHour, 0, 2);
    face.minutes.number = number(0, 59, MsPerMinute, 0, 2);

    return face;
}

}
