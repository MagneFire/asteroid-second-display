// SPDX-FileCopyrightText: 2023 Ed Beroset <beroset@ieee.org>
// SPDX-FileCopyrightText: 2023, 2026 Arseniy Movshev <dodoradio@outlook.com>
// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef BACKEND_CASIO_H
#define BACKEND_CASIO_H

#include <QDateTime>
#include <QList>
#include <array>
#include <cstdint>

#include "backend.h"

namespace SecondDisplay {

using CasioPacket = std::array<std::uint8_t, 7>;

struct CasioModel
{
    int timeSyncDelaySeconds;
    std::uint8_t blackBackground;
    std::uint8_t whiteBackground;
    QList<CasioPacket> timepieceSequence;
};

extern const CasioModel Koi;
extern const CasioModel Medaka;

CasioPacket buildCasioTimePacket(const QDateTime &localTime, int delaySeconds);
CasioPacket buildCasioTimeFormatPacket(TimeFormat format);
CasioPacket buildCasioBackgroundPacket(const CasioModel &model, Background background);

class CasioBackend : public Backend
{
    Q_OBJECT

public:
    explicit CasioBackend(const CasioModel &model, const QString &devicePath = "/dev/MultiSensors_CD_01",
                          QObject *parent = nullptr);

    unsigned int capabilities() const override;
    bool synchronizeTime(TimeFormat format) override;
    bool prepareTimepiece() override;
    bool setBackground(Background background) override;

private:
    bool write(const CasioPacket &packet) const;

    const CasioModel &m_model;
    QString m_devicePath;
};

}

#endif
