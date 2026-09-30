// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "display.h"

namespace SecondDisplay {

Display::Display(Backend *backend, QObject *parent)
    : QObject(parent)
    , m_backend(backend)
{
    connect(m_backend, &Backend::capabilitiesChanged, this, &Display::capabilitiesChanged);
}

uint Display::capabilities() const
{
    return m_backend->capabilities();
}

bool Display::supports(unsigned int capability) const
{
    return capabilities() & capability;
}

bool Display::applyFeature(bool &currentlyEnabled, bool enabled, unsigned int capability, bool (Backend::*setEnabled)(bool))
{
    if (currentlyEnabled == enabled || !supports(capability) || !(m_backend->*setEnabled)(enabled))
        return false;
    currentlyEnabled = enabled;
    return true;
}

bool Display::stepCounterEnabled() const
{
    return m_stepCounterEnabled;
}

void Display::setStepCounterEnabled(bool enabled)
{
    if (applyFeature(m_stepCounterEnabled, enabled, Capability::StepCounter, &Backend::setStepCounterEnabled))
        emit stepCounterEnabledChanged();
}

bool Display::heartRateEnabled() const
{
    return m_heartRateEnabled;
}

void Display::setHeartRateEnabled(bool enabled)
{
    if (applyFeature(m_heartRateEnabled, enabled, Capability::HeartRate, &Backend::setHeartRateEnabled))
        emit heartRateEnabledChanged();
}

bool Display::motionEnabled() const
{
    return m_motionEnabled;
}

void Display::setMotionEnabled(bool enabled)
{
    if (applyFeature(m_motionEnabled, enabled, Capability::Motion, &Backend::setMotionEnabled))
        emit motionEnabledChanged();
}

int Display::displayColor() const
{
    return static_cast<int>(m_background);
}

void Display::setDisplayColor(int color)
{
    const auto background = static_cast<Background>(color);
    if (background != Background::Black && background != Background::White)
        return;
    if (background == m_background || !supports(Capability::DisplayColor) || !m_backend->setBackground(background))
        return;
    m_background = background;
    emit displayColorChanged();
}

bool Display::SynchronizeTime()
{
    return supports(Capability::TimeSync) && m_backend->synchronizeTime(TimeFormat::TwentyFourHour);
}

bool Display::EnterTimepieceMode(bool)
{
    return supports(Capability::TimepieceMode) && m_backend->prepareTimepiece();
}

}
