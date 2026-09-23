// SPDX-FileCopyrightText: 2022 Cristian Le <github@lecris.me>
// SPDX-FileCopyrightText: 2023 Ed Beroset <beroset@ieee.org>
// SPDX-FileCopyrightText: 2023 Arseniy Movshev <dodoradio@outlook.com>
// SPDX-FileCopyrightText: 2022, 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "backend_mobvoi.h"

#include <QDebug>
#include <dlfcn.h>
#include <hybris/common/dlfcn.h>
#include <hybris/properties/properties.h>

namespace SecondDisplay {

namespace {

constexpr int LoadRetryIntervalMs = 5000;
constexpr int LoadRetries = 24;

}

MobvoiBackend::MobvoiBackend(const MobvoiModel &model, QObject *parent)
    : Backend(parent)
    , m_symbolPrefix(QString::fromLatin1(model.symbolPrefix))
    , m_hasTimepieceMode(model.hasTimepieceMode)
    , m_remainingRetries(LoadRetries)
{
    m_retryTimer.setInterval(LoadRetryIntervalMs);
    connect(&m_retryTimer, &QTimer::timeout, this, &MobvoiBackend::loadLibrary);
    loadLibrary();
}

MobvoiBackend::~MobvoiBackend()
{
    if (m_library)
        hybris_dlclose(m_library);
}

void MobvoiBackend::loadLibrary()
{
    m_library = hybris_dlopen("libmcutool.so", RTLD_LAZY);
    if (!m_library) {
        if (m_remainingRetries-- > 0) {
            m_retryTimer.start();
        } else {
            m_retryTimer.stop();
            qWarning() << "Unable to load libmcutool.so, giving up";
        }
        return;
    }
    m_retryTimer.stop();

    m_syncTime = resolve<Command>("SyncTime");
    m_cutOffScreen = resolve<Command>("CutOffScreen");
    m_wipeBandModeData = resolve<Command>("WipeBandModeData");
    m_bandMode = resolve<Command>("BandMode");
    m_autoLowPowerScreen = resolve<Switch>("AutoLowPowerScreen");
    m_enableStepCounter = resolve<Switch>("EnableStepCounter");
    m_enableHeartRate = resolve<Switch>("EnableHeartRate");
    m_enableMotion = resolve<Switch>("EnableMotion");
    emit capabilitiesChanged();
}

template<typename Function>
Function MobvoiBackend::resolve(const char *name) const
{
    const QByteArray symbol = (m_symbolPrefix + QLatin1String(name)).toLatin1();
    auto *function = reinterpret_cast<Function>(hybris_dlsym(m_library, symbol.constData()));
    if (!function)
        qWarning() << "libmcutool.so has no" << symbol;
    return function;
}

unsigned int MobvoiBackend::capabilities() const
{
    unsigned int capabilities = 0;
    if (m_syncTime)
        capabilities |= Capability::TimeSync;
    if (m_hasTimepieceMode && m_autoLowPowerScreen && m_cutOffScreen && m_wipeBandModeData && m_bandMode)
        capabilities |= Capability::TimepieceMode;
    // The sensor setters redraw the LCD through SyncTime afterwards.
    if (m_syncTime && m_enableStepCounter)
        capabilities |= Capability::StepCounter;
    if (m_syncTime && m_enableHeartRate)
        capabilities |= Capability::HeartRate;
    if (m_syncTime && m_enableMotion)
        capabilities |= Capability::Motion;
    return capabilities;
}

bool MobvoiBackend::logResult(const char *call, int result) const
{
    if (result != 0)
        qInfo() << "libmcutool" << call << "returned" << result;
    return true;
}

bool MobvoiBackend::redrawLcd() const
{
    return logResult("SyncTime", m_syncTime());
}

bool MobvoiBackend::synchronizeTime(TimeFormat format)
{
    property_set("persist.sys.time_12_24", format == TimeFormat::TwelveHour ? "12" : "24");
    return logResult("SyncTime", m_syncTime());
}

bool MobvoiBackend::prepareTimepiece()
{
    logResult("AutoLowPowerScreen on", m_autoLowPowerScreen(0, 0, true));
    logResult("AutoLowPowerScreen off", m_autoLowPowerScreen(0, 0, false));
    logResult("CutOffScreen", m_cutOffScreen());
    logResult("WipeBandModeData", m_wipeBandModeData());
    logResult("BandMode", m_bandMode());
    return true;
}

bool MobvoiBackend::setStepCounterEnabled(bool enabled)
{
    logResult("EnableStepCounter", m_enableStepCounter(0, 0, enabled));
    return redrawLcd();
}

bool MobvoiBackend::setHeartRateEnabled(bool enabled)
{
    logResult("EnableHeartRate", m_enableHeartRate(0, 0, enabled));
    return redrawLcd();
}

bool MobvoiBackend::setMotionEnabled(bool enabled)
{
    logResult("EnableMotion", m_enableMotion(0, 0, enabled));
    return redrawLcd();
}

}
