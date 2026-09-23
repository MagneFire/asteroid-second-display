// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "display.h"

#include "settingskeys.h"

namespace SecondDisplay {

struct Display::Feature
{
    bool Display::*enabled;
    const char *settingsKey;
    unsigned int capability;
    bool (Backend::*setEnabled)(bool);
    void (Display::*enabledChanged)();
};

const Display::Feature Display::StepCounter{&Display::m_stepCounterEnabled, SettingsKey::StepCounter,
                                            Capability::StepCounter, &Backend::setStepCounterEnabled,
                                            &Display::stepCounterEnabledChanged};
const Display::Feature Display::HeartRate{&Display::m_heartRateEnabled, SettingsKey::HeartRate, Capability::HeartRate,
                                          &Backend::setHeartRateEnabled, &Display::heartRateEnabledChanged};
const Display::Feature Display::Motion{&Display::m_motionEnabled, SettingsKey::Motion, Capability::Motion,
                                       &Backend::setMotionEnabled, &Display::motionEnabledChanged};

Display::Display(Backend *backend, SettingsStore *settings, QObject *parent)
    : QObject(parent)
    , m_backend(backend)
    , m_settings(settings)
{
    connect(m_backend, &Backend::capabilitiesChanged, this, &Display::capabilitiesChanged);
    connect(m_backend, &Backend::capabilitiesChanged, this, &Display::applyStoredSettings);
    connect(m_settings, &SettingsStore::valueChanged, this, [this](const QString &key) {
        if (key == QLatin1String(SettingsKey::Use12HourFormat))
            SynchronizeTime();
    });
    applyStoredSettings();
}

void Display::applyStoredSettings()
{
    for (const Feature *feature : {&StepCounter, &HeartRate, &Motion}) {
        const QVariant stored = m_settings->value(feature->settingsKey);
        if (stored.isValid())
            applyFeature(*feature, stored.toBool());
    }

    const QVariant storedColor = m_settings->value(SettingsKey::DisplayColor);
    if (storedColor.isValid())
        applyBackground(static_cast<Background>(storedColor.toInt()));

    SynchronizeTime();
}

uint Display::capabilities() const
{
    return m_backend->capabilities();
}

bool Display::supports(unsigned int capability) const
{
    return capabilities() & capability;
}

bool Display::applyFeature(const Feature &feature, bool enabled)
{
    if (!supports(feature.capability) || !(m_backend->*feature.setEnabled)(enabled))
        return false;
    if (this->*feature.enabled != enabled) {
        this->*feature.enabled = enabled;
        emit(this->*feature.enabledChanged)();
    }
    return true;
}

void Display::setFeatureEnabled(const Feature &feature, bool enabled)
{
    if (applyFeature(feature, enabled))
        m_settings->setValue(feature.settingsKey, enabled);
}

bool Display::applyBackground(Background background)
{
    if (background != Background::Black && background != Background::White)
        return false;
    if (!supports(Capability::DisplayColor) || !m_backend->setBackground(background))
        return false;
    if (background != m_background) {
        m_background = background;
        emit displayColorChanged();
    }
    return true;
}

bool Display::stepCounterEnabled() const
{
    return m_stepCounterEnabled;
}

void Display::setStepCounterEnabled(bool enabled)
{
    setFeatureEnabled(StepCounter, enabled);
}

bool Display::heartRateEnabled() const
{
    return m_heartRateEnabled;
}

void Display::setHeartRateEnabled(bool enabled)
{
    setFeatureEnabled(HeartRate, enabled);
}

bool Display::motionEnabled() const
{
    return m_motionEnabled;
}

void Display::setMotionEnabled(bool enabled)
{
    setFeatureEnabled(Motion, enabled);
}

int Display::displayColor() const
{
    return static_cast<int>(m_background);
}

void Display::setDisplayColor(int color)
{
    if (applyBackground(static_cast<Background>(color)))
        m_settings->setValue(SettingsKey::DisplayColor, color);
}

bool Display::SynchronizeTime()
{
    const bool twelveHour = m_settings->value(SettingsKey::Use12HourFormat).toBool();
    return supports(Capability::TimeSync)
        && m_backend->synchronizeTime(twelveHour ? TimeFormat::TwelveHour : TimeFormat::TwentyFourHour);
}

bool Display::EnterTimepieceMode(bool)
{
    return supports(Capability::TimepieceMode) && m_backend->prepareTimepiece();
}

}
