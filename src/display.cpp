// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "display.h"

#include "settingskeys.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

namespace {

const char MceService[] = "com.nokia.mce";
const char MceRequestPath[] = "/com/nokia/mce/request";
const char MceRequestInterface[] = "com.nokia.mce.request";
const char MceSignalPath[] = "/com/nokia/mce/signal";
const char MceSignalInterface[] = "com.nokia.mce.signal";
const char MceLowPowerModeKey[] = "/system/osso/dsm/display/use_low_power_mode";

}

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
const Display::Feature Display::AodOffload{&Display::m_aodOffloadEnabled, SettingsKey::AodOffload,
                                           Capability::AodOffload, &Backend::setAodOffloadEnabled,
                                           &Display::aodOffloadEnabledChanged};

Display::Display(Backend *backend, SettingsStore *settings, PowerOff powerOff, QObject *parent)
    : QObject(parent)
    , m_backend(backend)
    , m_settings(settings)
    , m_powerOff(std::move(powerOff))
{
    connect(m_backend, &Backend::capabilitiesChanged, this, &Display::capabilitiesChanged);
    connect(m_backend, &Backend::capabilitiesChanged, this, &Display::applyStoredSettings);
    connect(m_backend, &Backend::capabilitiesChanged, this, &Display::followDisplayStateIfNeeded);
    connect(m_backend, &Backend::aodOffloadActiveChanged, this, &Display::aodOffloadActiveChanged);
    connect(m_settings, &SettingsStore::valueChanged, this, [this](const QString &key) {
        if (key == QLatin1String(SettingsKey::Use12HourFormat))
            SynchronizeTime();
    });
    applyStoredSettings();
    followDisplayStateIfNeeded();
}

void Display::followDisplayStateIfNeeded()
{
    if (m_followingDisplayState || !supports(Capability::AodOffload))
        return;
    QDBusConnection bus = QDBusConnection::systemBus();
    m_followingDisplayState = bus.connect(MceService, MceSignalPath, MceSignalInterface, "display_status_ind", this,
                                          SLOT(onDisplayStatusChanged(QString)));
    bus.connect(MceService, MceSignalPath, MceSignalInterface, "config_change_ind", this,
                SLOT(onMceConfigChanged(QString, QDBusVariant)));

    QDBusMessage lowPowerMode = QDBusMessage::createMethodCall(MceService, MceRequestPath, MceRequestInterface,
                                                               "get_config");
    lowPowerMode << QVariant::fromValue(QDBusObjectPath(MceLowPowerModeKey));
    auto *configWatcher = new QDBusPendingCallWatcher(bus.asyncCall(lowPowerMode), this);
    connect(configWatcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<QDBusVariant> reply = *watcher;
        watcher->deleteLater();
        if (reply.isValid())
            onMceConfigChanged(MceLowPowerModeKey, reply.value());
    });

    QDBusMessage displayStatus = QDBusMessage::createMethodCall(MceService, MceRequestPath, MceRequestInterface,
                                                                "get_display_status");
    auto *statusWatcher = new QDBusPendingCallWatcher(bus.asyncCall(displayStatus), this);
    connect(statusWatcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<QString> reply = *watcher;
        watcher->deleteLater();
        if (reply.isValid())
            onDisplayStatusChanged(reply.value());
    });
}

void Display::onDisplayStatusChanged(const QString &state)
{
    m_backend->displayStateChanged(state);
}

void Display::onMceConfigChanged(const QString &key, const QDBusVariant &value)
{
    if (key == QLatin1String(MceLowPowerModeKey))
        m_backend->setAmbientEnabled(value.variant().toBool());
}

void Display::applyStoredSettings()
{
    for (const Feature *feature : {&StepCounter, &HeartRate, &Motion, &AodOffload}) {
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

bool Display::aodOffloadEnabled() const
{
    return m_aodOffloadEnabled;
}

void Display::setAodOffloadEnabled(bool enabled)
{
    setFeatureEnabled(AodOffload, enabled);
}

bool Display::aodOffloadActive() const
{
    return m_backend->aodOffloadActive();
}

bool Display::ReleaseAodOffload()
{
    return m_backend->releaseAodOffload();
}

bool Display::SetFace(const QVariantMap &description)
{
    FaceDescription face;
    face.backgroundPng = description.value("background").toString();
    face.digitsPng = description.value("digits").toString();
    face.minuteDigitsPng = description.value("minuteDigits").toString();
    face.colonPng = description.value("colon").toString();
    face.hours = QPoint(description.value("hoursX").toInt(), description.value("hoursY").toInt());
    face.minutes = QPoint(description.value("minutesX").toInt(), description.value("minutesY").toInt());
    face.colon = QPoint(description.value("colonX").toInt(), description.value("colonY").toInt());
    face.color = description.value("color").toBool();
    if (face.backgroundPng.isEmpty() || face.digitsPng.isEmpty())
        return false;
    return supports(Capability::AodOffload | Capability::TimepieceMode) && m_backend->setFace(face);
}

void Display::ClearFace()
{
    m_backend->clearFace();
}

bool Display::SynchronizeTime()
{
    const bool twelveHour = m_settings->value(SettingsKey::Use12HourFormat).toBool();
    return supports(Capability::TimeSync)
        && m_backend->synchronizeTime(twelveHour ? TimeFormat::TwelveHour : TimeFormat::TwentyFourHour);
}

bool Display::EnterTimepieceMode(bool powerOff)
{
    return supports(Capability::TimepieceMode) && m_backend->prepareTimepiece()
        && (!powerOff || (m_backend->enterTimepiece() && m_powerOff()));
}

}
