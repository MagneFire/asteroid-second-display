// SPDX-FileCopyrightText: 2022 Cristian Le <github@lecris.me>
// SPDX-FileCopyrightText: 2023 Ed Beroset <beroset@ieee.org>
// SPDX-FileCopyrightText: 2023 Arseniy Movshev <dodoradio@outlook.com>
// SPDX-FileCopyrightText: 2022, 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef BACKEND_MOBVOI_H
#define BACKEND_MOBVOI_H

#include <QTimer>
#include <cstdint>

#include "backend.h"

namespace SecondDisplay {

struct MobvoiModel
{
    const char *symbolPrefix;
    bool hasTimepieceMode;
};

inline constexpr MobvoiModel Catfish{"Java_com_mobvoi_ticwear_mcuservice_CoreService_native", true};
inline constexpr MobvoiModel Rubyfish{"Java_com_mobvoi_ticwear_mcuservice_backend_CoreService_native", false};

class MobvoiBackend : public Backend
{
    Q_OBJECT

public:
    explicit MobvoiBackend(const MobvoiModel &model, QObject *parent = nullptr);
    ~MobvoiBackend() override;

    unsigned int capabilities() const override;
    bool synchronizeTime(TimeFormat format) override;
    bool prepareTimepiece() override;
    bool setStepCounterEnabled(bool enabled) override;
    bool setHeartRateEnabled(bool enabled) override;
    bool setMotionEnabled(bool enabled) override;

private:
    using Command = int (*)();
    using Switch = int (*)(std::int32_t, std::int32_t, std::int32_t);

    void loadLibrary();
    template<typename Function>
    Function resolve(const char *name) const;
    bool logResult(const char *call, int result) const;
    bool redrawLcd() const;

    QString m_symbolPrefix;
    bool m_hasTimepieceMode;
    void *m_library = nullptr;
    QTimer m_retryTimer;
    int m_remainingRetries;

    Command m_syncTime = nullptr;
    Command m_cutOffScreen = nullptr;
    Command m_wipeBandModeData = nullptr;
    Command m_bandMode = nullptr;
    Switch m_autoLowPowerScreen = nullptr;
    Switch m_enableStepCounter = nullptr;
    Switch m_enableHeartRate = nullptr;
    Switch m_enableMotion = nullptr;
};

}

#endif
