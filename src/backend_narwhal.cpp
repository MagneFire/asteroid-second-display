// SPDX-FileCopyrightText: 2022-2023 Arseniy Movshev <dodoradio@outlook.com>
// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backend_narwhal.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>

namespace SecondDisplay {

QString sop716Time(const QDateTime &localTime)
{
    return localTime.toString("yyyy-MM-dd HH:mm:ss");
}

QString sop716UtcOffsetMinutes(const QDateTime &localTime)
{
    return QString::number(localTime.offsetFromUtc() / 60);
}

NarwhalBackend::NarwhalBackend(const QString &sysfsPath, QObject *parent)
    : Backend(parent)
    , m_sysfsPath(sysfsPath)
{
    QFile watchModeFile(attributePath("watch_mode"));
    m_watchMode = watchModeFile.open(QIODevice::ReadOnly) && watchModeFile.readAll().trimmed() == "1";
}

QString NarwhalBackend::attributePath(const char *attribute) const
{
    return m_sysfsPath + '/' + QLatin1String(attribute);
}

bool NarwhalBackend::isWritable(const char *attribute) const
{
    return QFileInfo(attributePath(attribute)).isWritable();
}

bool NarwhalBackend::write(const char *attribute, const QString &value) const
{
    QFile file(attributePath(attribute));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        qWarning() << "Unable to open" << file.fileName() << ":" << file.errorString();
        return false;
    }
    const QByteArray data = value.toLatin1();
    if (file.write(data) != data.size()) {
        qWarning() << "Unable to write" << value << "to" << file.fileName() << ":" << file.errorString();
        return false;
    }
    return true;
}

unsigned int NarwhalBackend::capabilities() const
{
    unsigned int capabilities = 0;
    if (isWritable("motor_move") && isWritable("motor_move_all") && isWritable("motor_init") && isWritable("watch_mode"))
        capabilities |= Capability::Hands;
    if (isWritable("time") && isWritable("tz_minutes"))
        capabilities |= Capability::TimeSync;
    return capabilities;
}

bool NarwhalBackend::synchronizeTime(TimeFormat)
{
    const QDateTime now = QDateTime::currentDateTime();
    if (!write("time", sop716Time(now)))
        return false;
    // Writing the time moves the hands to it and puts the driver back in watch mode.
    setWatchMode(true);
    return write("tz_minutes", sop716UtcOffsetMinutes(now));
}

bool NarwhalBackend::watchMode() const
{
    return m_watchMode;
}

void NarwhalBackend::setWatchMode(bool watchMode)
{
    if (m_watchMode == watchMode)
        return;
    m_watchMode = watchMode;
    emit watchModeChanged();
}

unsigned int NarwhalBackend::handResolution() const
{
    return HandResolution;
}

bool NarwhalBackend::resumeWatchMode()
{
    if (m_watchMode)
        return true;
    if (!write("watch_mode", "1"))
        return false;
    setWatchMode(true);
    return true;
}

QList<int> NarwhalBackend::handPositions() const
{
    QFile file(attributePath("position"));
    if (!file.open(QIODevice::ReadOnly))
        return {};
    const QList<QByteArray> fields = file.readAll().trimmed().split(':');
    if (fields.size() != 2)
        return {};
    return {fields[0].toInt(), fields[1].toInt()};
}

bool NarwhalBackend::moveHandsWith(const char *attribute, const QString &value)
{
    if (!write(attribute, value))
        return false;
    setWatchMode(false);
    return true;
}

bool NarwhalBackend::moveHand(Hand hand, int position)
{
    return moveHandsWith("motor_move", QStringLiteral("%1:%2").arg(static_cast<int>(hand)).arg(position));
}

bool NarwhalBackend::moveAllHands(const QList<int> &positions)
{
    return moveHandsWith("motor_move_all", QStringLiteral("%1:%2").arg(positions[0]).arg(positions[1]));
}

bool NarwhalBackend::calibrateHand(Hand hand, Rotation rotation, int steps)
{
    return moveHandsWith("motor_init", QStringLiteral("%1:%2:%3")
                                           .arg(static_cast<int>(hand))
                                           .arg(static_cast<int>(rotation))
                                           .arg(steps));
}

}
