// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "face.h"

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
constexpr std::int32_t MinuteFontId = 6;

bool pngSize(const QByteArray &png, std::int32_t *width, std::int32_t *height)
{
    if (png.size() < 24 || !png.startsWith("\x89PNG"))
        return false;
    *width = qFromBigEndian<quint32>(png.constData() + 16);
    *height = qFromBigEndian<quint32>(png.constData() + 20);
    return true;
}

std::optional<QByteArray> readPng(const QString &path, std::int32_t *width, std::int32_t *height)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to read" << path;
        return std::nullopt;
    }
    const QByteArray png = file.readAll();
    if (!pngSize(png, width, height)) {
        qWarning() << path << "is not a PNG";
        return std::nullopt;
    }
    return png;
}

DrawableInfo drawable(std::int32_t id, DrawableType type, QPoint position, int width, int height, int zOrder)
{
    DrawableInfo info;
    info.id = id;
    info.type = type;
    info.offsetX = position.x();
    info.offsetY = position.y();
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

NumberInfo number(std::int32_t fontId, int lowest, int highest, float msPerIncrement, long long timeOffsetMs,
                  int minDigitsShown)
{
    NumberInfo info;
    info.startTime = timeFromOffset(-timeOffsetMs);
    info.startNumber = lowest;
    info.minNumber = lowest;
    info.maxNumber = highest;
    info.increment = 1;
    info.msPerIncrement = msPerIncrement;
    info.fontId = fontId;
    info.leadingZeroes = std::max(0, minDigitsShown - 1);
    const int needed = highest <= 0 ? 1 : static_cast<int>(std::log(highest) / std::log(Glyphs)) + 1;
    info.digits = std::max(minDigitsShown, needed);
    info.digit = -1;
    info.transitionMs = 0;
    return info;
}

std::optional<Face::Font> loadFont(const QString &path, std::int32_t id)
{
    std::int32_t width = 0;
    std::int32_t height = 0;
    const auto png = readPng(path, &width, &height);
    if (!png)
        return std::nullopt;
    if (height % Glyphs != 0) {
        qWarning() << path << "is not a strip of" << Glyphs << "glyphs";
        return std::nullopt;
    }
    Face::Font font;
    font.info.id = id;
    font.info.width = width;
    font.info.height = height / Glyphs;
    font.info.nGlyphs = Glyphs;
    font.png = *png;
    return font;
}

}

QList<std::uint32_t> Face::ids() const
{
    QList<std::uint32_t> ids{static_cast<std::uint32_t>(font.info.id),
                             static_cast<std::uint32_t>(background.drawable.id),
                             static_cast<std::uint32_t>(hours.drawable.id),
                             static_cast<std::uint32_t>(minutes.drawable.id)};
    if (minuteFont)
        ids.append(static_cast<std::uint32_t>(minuteFont->info.id));
    if (colon)
        ids.append(static_cast<std::uint32_t>(colon->drawable.id));
    return ids;
}

std::optional<FaceDescription> defaultFaceDescription(const QString &directory, int displayWidth)
{
    FaceDescription description;
    description.backgroundPng = directory + "/background.png";
    description.digitsPng = directory + "/digits.png";
    description.colonPng = directory + "/colon.png";

    std::int32_t glyphWidth = 0;
    std::int32_t stripHeight = 0;
    std::int32_t colonWidth = 0;
    std::int32_t colonHeight = 0;
    if (!readPng(description.digitsPng, &glyphWidth, &stripHeight) || !readPng(description.colonPng, &colonWidth, &colonHeight))
        return std::nullopt;

    const float scale = static_cast<float>(displayWidth) / DesignSize;
    const int top = static_cast<int>(DigitsTop * scale);
    const int centre = displayWidth / 2;
    const int gap = static_cast<int>(Gap * scale) + colonWidth / 2;
    description.colon = QPoint(centre - colonWidth / 2, top);
    description.hours = QPoint(centre - gap - 2 * glyphWidth, top);
    description.minutes = QPoint(centre + gap, top);
    return description;
}

std::optional<Face> loadFace(const FaceDescription &description, bool twelveHour)
{
    Face face;
    std::int32_t width = 0;
    std::int32_t height = 0;

    const auto font = loadFont(description.digitsPng, FontId);
    if (!font)
        return std::nullopt;
    face.font = *font;
    if (!description.minuteDigitsPng.isEmpty()) {
        face.minuteFont = loadFont(description.minuteDigitsPng, MinuteFontId);
        if (!face.minuteFont)
            return std::nullopt;
    }
    const FontInfo &minuteInfo = face.minuteFont ? face.minuteFont->info : face.font.info;

    const auto background = readPng(description.backgroundPng, &width, &height);
    if (!background)
        return std::nullopt;
    face.background.drawable = drawable(BackgroundId, DrawableType::Generic, QPoint(0, 0), width, height, 0);
    face.background.png = *background;

    if (!description.colonPng.isEmpty()) {
        const auto colon = readPng(description.colonPng, &width, &height);
        if (!colon)
            return std::nullopt;
        face.colon = Face::Bitmap{drawable(ColonId, DrawableType::Generic, description.colon, width, height, 1), *colon};
    }

    face.hours.drawable = drawable(HoursId, DrawableType::Number, description.hours, 2 * face.font.info.width,
                                   face.font.info.height, 2);
    face.minutes.drawable = drawable(MinutesId, DrawableType::Number, description.minutes, 2 * minuteInfo.width,
                                     minuteInfo.height, 3);
    if (twelveHour)
        face.hours.number = number(FontId, 1, 12, MsPerHour, -static_cast<long long>(MsPerHour), 1);
    else
        face.hours.number = number(FontId, 0, 23, MsPerHour, 0, 2);
    face.minutes.number = number(minuteInfo.id, 0, 59, MsPerMinute, 0, 2);

    return face;
}

}
