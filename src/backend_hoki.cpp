// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "backend_hoki.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QDir>

namespace SecondDisplay {

namespace {

constexpr int ConnectRetryIntervalMs = 5000;
constexpr std::int16_t TimepieceBrightness = 128;
constexpr std::int16_t TimepieceBrightnessDim = 40;
constexpr std::uint32_t TimepieceBrightMs = 5000;
constexpr bool TimepieceTiltToBright = true;

}

HokiBackend::HokiBackend(const QString &faceDirectory, QObject *parent)
    : Backend(parent)
    , m_faceDirectory(faceDirectory)
{
    connect(&m_client, &SidekickClient::connected, this, &HokiBackend::onConnected);
    connect(&m_client, &SidekickClient::disconnected, this, &HokiBackend::onDisconnected);
    m_retryTimer.setInterval(ConnectRetryIntervalMs);
    connect(&m_retryTimer, &QTimer::timeout, this, &HokiBackend::connectToService);
    connectToService();
}

void HokiBackend::connectToService()
{
    if (m_client.connectToService())
        m_retryTimer.stop();
    else
        m_retryTimer.start();
}

void HokiBackend::onConnected()
{
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
    emit capabilitiesChanged();
    m_retryTimer.start();
}

unsigned int HokiBackend::capabilities() const
{
    if (!m_capabilities)
        return 0;
    unsigned int capabilities = Capability::TimeSync;
    if (QDir(m_faceDirectory).exists("digits.png"))
        capabilities |= Capability::TimepieceMode;
    return capabilities;
}

bool HokiBackend::synchronizeTime(TimeFormat format)
{
    m_format = format;
    return m_client.updateDisplayTime().ok();
}

bool HokiBackend::uploadFace(const TimepieceFace &face)
{
    if (!m_client.beginResources().ok())
        return false;
    const bool sent = m_client.sendFontPng8888(face.font, face.fontPng).ok()
        && m_client.sendBitmapPng8888(face.background.drawable, face.background.png).ok()
        && m_client.sendBitmapPng8888(face.colon.drawable, face.colon.png).ok()
        && m_client.sendNumberResource(face.hours.drawable, face.hours.number).ok()
        && m_client.sendNumberResource(face.minutes.drawable, face.minutes.number).ok();
    const bool ok = m_client.endResources().ok() && sent;
    if (!ok)
        m_client.reset();
    return ok;
}

bool HokiBackend::configureTimepiece()
{
    if (!m_client.setBrightness(true, {}, {}, {TimepieceBrightness}, {TimepieceBrightnessDim}).ok())
        return false;
    m_client.setAlsMode(Sidekick::AlsMode::Off, 0, 0);
    return m_client.setTwmConfig(TimepieceBrightMs, TimepieceTiltToBright).ok();
}

bool HokiBackend::prepareTimepiece()
{
    if (!m_capabilities)
        return false;
    const auto face = loadTimepieceFace(m_faceDirectory, m_format == TimeFormat::TwelveHour,
                                        static_cast<int>(m_capabilities->displayWidth));
    if (!face)
        return false;
    if (!m_client.reset().ok() || !m_client.setColorFormat(Sidekick::ColorFormat::Gray).ok() || !uploadFace(*face)
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
