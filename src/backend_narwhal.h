// SPDX-FileCopyrightText: 2022-2023 Arseniy Movshev <dodoradio@outlook.com>
// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKEND_NARWHAL_H
#define BACKEND_NARWHAL_H

#include <QDateTime>

#include "backend.h"

namespace SecondDisplay {

QString sop716Time(const QDateTime &localTime);
QString sop716UtcOffsetMinutes(const QDateTime &localTime);

class NarwhalBackend : public Backend
{
    Q_OBJECT

public:
    static constexpr int HandResolution = 180;

    explicit NarwhalBackend(const QString &sysfsPath = "/sys/devices/sop716", QObject *parent = nullptr);

    unsigned int capabilities() const override;
    bool synchronizeTime(TimeFormat format) override;

    bool watchMode() const override;
    unsigned int handResolution() const override;
    bool resumeWatchMode() override;
    QList<int> handPositions() const override;
    bool moveHand(Hand hand, int position) override;
    bool moveAllHands(const QList<int> &positions) override;
    bool calibrateHand(Hand hand, Rotation rotation, int steps) override;

private:
    QString attributePath(const char *attribute) const;
    bool isWritable(const char *attribute) const;
    bool write(const char *attribute, const QString &value) const;
    bool moveHandsWith(const char *attribute, const QString &value);
    void setWatchMode(bool watchMode);

    QString m_sysfsPath;
    bool m_watchMode;
};

}

#endif
