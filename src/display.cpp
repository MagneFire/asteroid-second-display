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

bool Display::SynchronizeTime()
{
    return supports(Capability::TimeSync) && m_backend->synchronizeTime(TimeFormat::TwentyFourHour);
}

bool Display::EnterTimepieceMode(bool)
{
    return supports(Capability::TimepieceMode) && m_backend->prepareTimepiece();
}

}
