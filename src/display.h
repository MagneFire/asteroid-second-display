// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DISPLAY_H
#define DISPLAY_H

#include <QObject>

#include "backend.h"

namespace SecondDisplay {

class Display : public QObject
{
    Q_OBJECT
    Q_PROPERTY(uint Capabilities READ capabilities NOTIFY capabilitiesChanged)
    Q_PROPERTY(bool StepCounterEnabled READ stepCounterEnabled WRITE setStepCounterEnabled NOTIFY stepCounterEnabledChanged)
    Q_PROPERTY(bool HeartRateEnabled READ heartRateEnabled WRITE setHeartRateEnabled NOTIFY heartRateEnabledChanged)
    Q_PROPERTY(bool MotionEnabled READ motionEnabled WRITE setMotionEnabled NOTIFY motionEnabledChanged)
    Q_PROPERTY(int DisplayColor READ displayColor WRITE setDisplayColor NOTIFY displayColorChanged)

public:
    explicit Display(Backend *backend, QObject *parent = nullptr);

    uint capabilities() const;

    bool stepCounterEnabled() const;
    void setStepCounterEnabled(bool enabled);
    bool heartRateEnabled() const;
    void setHeartRateEnabled(bool enabled);
    bool motionEnabled() const;
    void setMotionEnabled(bool enabled);
    int displayColor() const;
    void setDisplayColor(int color);

public slots:
    bool SynchronizeTime();
    bool EnterTimepieceMode(bool powerOff);

signals:
    void capabilitiesChanged();
    void stepCounterEnabledChanged();
    void heartRateEnabledChanged();
    void motionEnabledChanged();
    void displayColorChanged();

private:
    bool supports(unsigned int capability) const;
    bool applyFeature(bool &currentlyEnabled, bool enabled, unsigned int capability, bool (Backend::*setEnabled)(bool));

    Backend *m_backend;
    bool m_stepCounterEnabled = false;
    bool m_heartRateEnabled = false;
    bool m_motionEnabled = false;
    Background m_background = Background::Black;
};

}

#endif
