// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "backend_hoki.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QDir>
#include <QThread>

namespace SecondDisplay {

namespace {

constexpr std::int16_t FixedBrightness = 128;
constexpr std::int16_t FixedBrightnessDim = 40;
constexpr float AlsAlpha = 80;
constexpr float DimScale = 0.6f;
const QList<std::int16_t> AlsThresholdsLux{25, 50, 100, 200, 400, 500, 800, 1000, 1600, 3000, 5000, 10000};
const QList<std::int16_t> AlsBrightness{39, 70, 79, 101, 118, 130, 138, 151, 170, 180, 192, 209, 224};
constexpr std::uint32_t TimepieceBrightMs = 5000;
constexpr bool TimepieceTiltToBright = true;
constexpr int BeginDisplayRetries = 20;
constexpr int BeginDisplayRetryMs = 100;

}

HokiBackend::HokiBackend(const QString &faceDirectory, QObject *parent)
    : Backend(parent)
    , m_faceDirectory(faceDirectory)
{
    connect(&m_client, &SidekickClient::connected, this, &HokiBackend::onConnected);
    connect(&m_client, &SidekickClient::disconnected, this, &HokiBackend::onDisconnected);
    m_client.connectToService();
}

void HokiBackend::onConnected()
{
    m_client.endDisplay(Sidekick::DisplayPowerState::Full);
    m_capabilities = m_client.getCapabilities();
    if (m_capabilities) {
        qInfo().nospace() << "Sidekick capabilities 0x" << Qt::hex << m_capabilities->capabilities << Qt::dec << ", "
                          << m_capabilities->displayWidth << "x" << m_capabilities->displayHeight << ", "
                          << m_capabilities->bytesAvailable << " bytes available";
    }
    emit capabilitiesChanged();
}

void HokiBackend::onDisconnected()
{
    m_capabilities.reset();
    m_timepiecePrepared = false;
    m_faceLoaded = false;
    setOffloadActive(false);
    emit capabilitiesChanged();
}

unsigned int HokiBackend::capabilities() const
{
    if (!m_capabilities)
        return 0;
    unsigned int capabilities = Capability::TimeSync;
    if (QDir(m_faceDirectory).exists("digits.png"))
        capabilities |= Capability::TimepieceMode | Capability::AodOffload;
    return capabilities;
}

bool HokiBackend::synchronizeTime(TimeFormat format)
{
    const bool formatChanged = format != m_format;
    m_format = format;
    if (formatChanged)
        reloadFace();
    return m_client.updateDisplayTime().ok();
}

std::optional<Face> HokiBackend::buildFace() const
{
    if (!m_capabilities)
        return std::nullopt;
    const int displayWidth = static_cast<int>(m_capabilities->displayWidth);
    const auto description = m_face ? m_face : defaultFaceDescription(m_faceDirectory, displayWidth);
    if (!description)
        return std::nullopt;
    return loadFace(*description, m_format == TimeFormat::TwelveHour);
}

bool HokiBackend::setFace(const FaceDescription &description)
{
    if (!loadFace(description, m_format == TimeFormat::TwelveHour))
        return false;
    m_face = description;
    reloadFace();
    return true;
}

void HokiBackend::clearFace()
{
    if (!m_face)
        return;
    m_face.reset();
    reloadFace();
}

void HokiBackend::reloadFace()
{
    if (!m_faceLoaded)
        return;
    const bool active = m_offloadActive;
    if (active)
        stopOffload();
    m_faceLoaded = false;
    if (active)
        startOffload();
}

Sidekick::ColorFormat HokiBackend::colorFormat() const
{
    return m_face && m_face->color ? Sidekick::ColorFormat::Rgb565 : Sidekick::ColorFormat::Gray;
}

bool HokiBackend::ensureFaceLoaded()
{
    if (m_faceLoaded)
        return true;
    const auto face = buildFace();
    if (!face)
        return false;
    if (!m_client.reset().ok() || !m_client.setColorFormat(colorFormat()).ok() || !uploadFace(*face))
        return false;
    if (!applyBrightness())
        return false;
    m_faceLoaded = true;
    return true;
}

bool HokiBackend::applyBrightness()
{
    if (m_alsEnabled) {
        QList<std::int16_t> dim;
        for (std::int16_t level : AlsBrightness)
            dim.append(static_cast<std::int16_t>(level * DimScale));
        if (!m_client.setBrightness(true, AlsThresholdsLux, AlsThresholdsLux, AlsBrightness, dim).ok())
            return false;
        return m_client.setAlsMode(Sidekick::AlsMode::On, AlsAlpha, AlsAlpha).ok();
    }
    if (!m_client.setBrightness(true, {}, {}, {FixedBrightness}, {FixedBrightnessDim}).ok())
        return false;
    return m_client.setAlsMode(Sidekick::AlsMode::Off, AlsAlpha, AlsAlpha).ok();
}

void HokiBackend::setAmbientLightSensorEnabled(bool enabled)
{
    if (m_alsEnabled == enabled)
        return;
    m_alsEnabled = enabled;
    if (m_faceLoaded)
        applyBrightness();
}

bool HokiBackend::setAodOffloadEnabled(bool enabled)
{
    m_offloadEnabled = enabled;
    if (enabled && m_displayOff)
        startOffload();
    else if (!enabled)
        stopOffload();
    return true;
}

void HokiBackend::setAmbientEnabled(bool enabled)
{
    m_ambientEnabled = enabled;
    if (enabled && m_displayOff)
        startOffload();
    else if (!enabled)
        stopOffload();
}

void HokiBackend::displayStateChanged(const QString &state)
{
    m_displayOff = state == QLatin1String("off");
    if (m_displayOff)
        startOffload();
    else
        stopOffload();
}

bool HokiBackend::aodOffloadActive() const
{
    return m_offloadActive;
}

bool HokiBackend::releaseAodOffload()
{
    stopOffload();
    return !m_offloadActive;
}

void HokiBackend::startOffload()
{
    if (m_offloadActive || !m_offloadEnabled || !m_ambientEnabled || !m_capabilities || m_timepiecePrepared)
        return;
    if (!m_face || !ensureFaceLoaded())
        return;
    m_client.updateDisplayTime();
    for (int attempt = 0; attempt < BeginDisplayRetries; ++attempt) {
        if (m_client.beginDisplay(Sidekick::DisplayPowerState::Full, attempt == BeginDisplayRetries - 1).ok()) {
            setOffloadActive(true);
            return;
        }
        QThread::msleep(BeginDisplayRetryMs);
    }
    qWarning() << "The BG did not take the display over";
}

void HokiBackend::stopOffload()
{
    if (!m_offloadActive)
        return;
    m_client.endDisplay(Sidekick::DisplayPowerState::Full);
    setOffloadActive(false);
}

void HokiBackend::setOffloadActive(bool active)
{
    if (m_offloadActive == active)
        return;
    m_offloadActive = active;
    qInfo() << "AOD offload" << (active ? "active" : "stopped");
    emit aodOffloadActiveChanged();
}

bool HokiBackend::uploadFace(const Face &face)
{
    if (!m_client.beginResources().ok())
        return false;
    const bool sent = m_client.sendFontPng8888(face.font.info, face.font.png).ok()
        && (!face.minuteFont || m_client.sendFontPng8888(face.minuteFont->info, face.minuteFont->png).ok())
        && m_client.sendBitmapPng8888(face.background.drawable, face.background.png).ok()
        && (!face.colon || m_client.sendBitmapPng8888(face.colon->drawable, face.colon->png).ok())
        && m_client.sendNumberResource(face.hours.drawable, face.hours.number).ok()
        && m_client.sendNumberResource(face.minutes.drawable, face.minutes.number).ok();
    const bool ok = m_client.endResources().ok() && sent;
    if (!ok)
        m_client.reset();
    return ok;
}

bool HokiBackend::configureTimepiece()
{
    if (!applyBrightness())
        return false;
    return m_client.setTwmConfig(TimepieceBrightMs, TimepieceTiltToBright).ok();
}

bool HokiBackend::prepareTimepiece()
{
    const auto face = buildFace();
    if (!face)
        return false;
    stopOffload();
    m_faceLoaded = false;
    if (!m_client.reset().ok() || !m_client.setColorFormat(colorFormat()).ok() || !uploadFace(*face)
        || !configureTimepiece() || !m_client.prepareTwm().ok())
        return false;
    m_timepiecePrepared = true;
    return true;
}

bool HokiBackend::blankDisplay() const
{
    QDBusMessage request = QDBusMessage::createMethodCall("com.nokia.mce", "/com/nokia/mce/request",
                                                          "com.nokia.mce.request", "req_display_state_off");
    const QDBusMessage reply = QDBusConnection::systemBus().call(request);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "mce req_display_state_off failed:" << reply.errorMessage();
        return false;
    }
    return true;
}

bool HokiBackend::enterTimepiece()
{
    if (!m_timepiecePrepared)
        return false;
    blankDisplay();
    if (!m_client.enterTwm().ok()) {
        qWarning() << "enterTwm failed, the BG stays on the time-only firmware until the next boot";
        return false;
    }
    return true;
}

}
