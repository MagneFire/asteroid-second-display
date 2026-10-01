// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DISPLAY_H
#define DISPLAY_H

#include <QObject>
#include <functional>

#include "backend.h"
#include "settingsstore.h"

namespace SecondDisplay {

class Display : public QObject
{
    Q_OBJECT
    Q_PROPERTY(uint Capabilities READ capabilities NOTIFY capabilitiesChanged)
    Q_PROPERTY(bool StepCounterEnabled READ stepCounterEnabled WRITE setStepCounterEnabled NOTIFY stepCounterEnabledChanged)
    Q_PROPERTY(bool HeartRateEnabled READ heartRateEnabled WRITE setHeartRateEnabled NOTIFY heartRateEnabledChanged)
    Q_PROPERTY(bool MotionEnabled READ motionEnabled WRITE setMotionEnabled NOTIFY motionEnabledChanged)
    Q_PROPERTY(int DisplayColor READ displayColor WRITE setDisplayColor NOTIFY displayColorChanged)
    Q_PROPERTY(bool AodOffloadEnabled READ aodOffloadEnabled WRITE setAodOffloadEnabled NOTIFY aodOffloadEnabledChanged)

public:
    using PowerOff = std::function<bool()>;

    Display(Backend *backend, SettingsStore *settings, PowerOff powerOff, QObject *parent = nullptr);

    uint capabilities() const;

    bool stepCounterEnabled() const;
    void setStepCounterEnabled(bool enabled);
    bool heartRateEnabled() const;
    void setHeartRateEnabled(bool enabled);
    bool motionEnabled() const;
    void setMotionEnabled(bool enabled);
    int displayColor() const;
    void setDisplayColor(int color);
    bool aodOffloadEnabled() const;
    void setAodOffloadEnabled(bool enabled);

public slots:
    bool SynchronizeTime();
    bool EnterTimepieceMode(bool powerOff);

signals:
    void capabilitiesChanged();
    void stepCounterEnabledChanged();
    void heartRateEnabledChanged();
    void motionEnabledChanged();
    void displayColorChanged();
    void aodOffloadEnabledChanged();

private slots:
    void onDisplayStatusChanged(const QString &state);

private:
    struct Feature;
    static const Feature StepCounter;
    static const Feature HeartRate;
    static const Feature Motion;
    static const Feature AodOffload;

    bool supports(unsigned int capability) const;
    bool applyFeature(const Feature &feature, bool enabled);
    void setFeatureEnabled(const Feature &feature, bool enabled);
    bool applyBackground(Background background);
    void applyStoredSettings();
    void followDisplayStateIfNeeded();

    Backend *m_backend;
    SettingsStore *m_settings;
    PowerOff m_powerOff;
    bool m_followingDisplayState = false;
    bool m_stepCounterEnabled = false;
    bool m_heartRateEnabled = false;
    bool m_motionEnabled = false;
    Background m_background = Background::Black;
    bool m_aodOffloadEnabled = false;
};

}

#endif
