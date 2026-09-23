// SPDX-FileCopyrightText: 2023 Ed Beroset <beroset@ieee.org>
// SPDX-FileCopyrightText: 2023, 2026 Arseniy Movshev <dodoradio@outlook.com>
// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "backend_casio.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>

namespace SecondDisplay {

namespace {

constexpr std::uint8_t SetTimeCommand = 0x50;
constexpr std::uint8_t SetSettingCommand = 0xFE;
constexpr std::uint8_t SetSettingWrite = 0x01;
constexpr std::uint8_t TimeFormatSetting = 0x01;
constexpr std::uint8_t BackgroundSetting = 0x05;
constexpr std::uint8_t TwentyFourHourFlag = 0x01;
constexpr std::uint8_t TwelveHourFlag = 0x00;

int sundayBasedWeekday(const QDate &date)
{
    return date.dayOfWeek() % 7;
}

}

const CasioModel Koi{1, 0x00, 0x01, {{0x02, 0xC1, 0xBE, 0x78, 0x3F, 0x91, 0xC7}}};

const CasioModel Medaka{0, 0x02, 0x01,
                        {{0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, {0xFE, 0x81, 0x01, 0x00, 0x00, 0x00, 0x00}}};

CasioPacket buildCasioTimePacket(const QDateTime &localTime, int delaySeconds)
{
    const QDate date = localTime.date();
    const QTime time = localTime.time();
    const int yearsSince2000 = date.year() - 2000;
    return {
        SetTimeCommand,
        0x00,
        static_cast<std::uint8_t>(((yearsSince2000 & 0x7F) << 1) | ((date.month() & 0x08) >> 3)),
        static_cast<std::uint8_t>((date.month() << 5) | (date.day() & 0x1F)),
        static_cast<std::uint8_t>((sundayBasedWeekday(date) << 5) | (time.hour() & 0x1F)),
        static_cast<std::uint8_t>(time.minute()),
        static_cast<std::uint8_t>(time.second() + delaySeconds),
    };
}

CasioPacket buildCasioTimeFormatPacket(TimeFormat format)
{
    const std::uint8_t flag = format == TimeFormat::TwentyFourHour ? TwentyFourHourFlag : TwelveHourFlag;
    return {SetSettingCommand, SetSettingWrite, TimeFormatSetting, flag, 0x00, 0x00, 0x00};
}

CasioPacket buildCasioBackgroundPacket(const CasioModel &model, Background background)
{
    const std::uint8_t value = background == Background::White ? model.whiteBackground : model.blackBackground;
    return {SetSettingCommand, SetSettingWrite, BackgroundSetting, value, 0x00, 0x00, 0x00};
}

CasioBackend::CasioBackend(const CasioModel &model, const QString &devicePath, QObject *parent)
    : Backend(parent)
    , m_model(model)
    , m_devicePath(devicePath)
{
}

unsigned int CasioBackend::capabilities() const
{
    if (!QFileInfo(m_devicePath).isWritable())
        return 0;
    return Capability::TimeSync | Capability::TimepieceMode | Capability::DisplayColor;
}

bool CasioBackend::write(const CasioPacket &packet) const
{
    QFile device(m_devicePath);
    if (!device.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        qWarning() << "Unable to open" << m_devicePath << ":" << device.errorString();
        return false;
    }
    const qint64 written = device.write(reinterpret_cast<const char *>(packet.data()), packet.size());
    if (written != static_cast<qint64>(packet.size())) {
        qWarning() << "Unable to write to" << m_devicePath << ":" << device.errorString();
        return false;
    }
    return true;
}

bool CasioBackend::synchronizeTime(TimeFormat format)
{
    return write(buildCasioTimeFormatPacket(format))
        && write(buildCasioTimePacket(QDateTime::currentDateTime(), m_model.timeSyncDelaySeconds));
}

bool CasioBackend::prepareTimepiece()
{
    for (const CasioPacket &packet : m_model.timepieceSequence) {
        if (!write(packet))
            return false;
    }
    return true;
}

bool CasioBackend::setBackground(Background background)
{
    return write(buildCasioBackgroundPacket(m_model, background));
}

}
