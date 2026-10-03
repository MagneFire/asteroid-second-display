// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SIDEKICKCLIENT_H
#define SIDEKICKCLIENT_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <cstdint>
#include <optional>

#include "sidekicktypes.h"

struct gbinder_servicemanager;
struct gbinder_remote_object;
struct gbinder_client;
struct gbinder_local_request;
struct gbinder_remote_reply;
struct gbinder_reader;

namespace SecondDisplay {

class SidekickClient : public QObject
{
    Q_OBJECT

public:
    struct Result
    {
        int binderStatus = -1;
        Sidekick::Status status = Sidekick::Status::UnknownError;

        bool ok() const;
        QString toString() const;
    };

    struct Capabilities
    {
        std::uint32_t capabilities = 0;
        Sidekick::ColorCapability color;
        std::uint32_t bytesAvailable = 0;
        std::uint32_t displayWidth = 0;
        std::uint32_t displayHeight = 0;
    };

    explicit SidekickClient(const QString &serviceName = QString::fromLatin1(Sidekick::DefaultService),
                            QObject *parent = nullptr);
    ~SidekickClient() override;

    bool connectToService();
    bool isConnected() const;
    QString serviceName() const;

    static QStringList listServices();

    std::optional<Capabilities> getCapabilities();
    std::optional<std::uint32_t> getBytesAvailable();
    std::optional<Sidekick::ColorFormat> getColorFormat();
    Result setColorFormat(Sidekick::ColorFormat format);
    std::optional<QByteArray> readFramebuffer();
    Result reset();

    Result beginResources();
    Result endResources();
    Result deleteResources(const QList<std::uint32_t> &ids);
    Result sendBitmapPng8888(const Sidekick::DrawableInfo &drawable, const QByteArray &png);
    Result sendFontPng8888(const Sidekick::FontInfo &font, const QByteArray &png);
    Result sendNumberResource(const Sidekick::DrawableInfo &drawable, const Sidekick::NumberInfo &number);

    Result updateDisplayTime();
    Result setBrightness(bool discrete, const QList<std::int16_t> &alsThresholdsUp,
                         const QList<std::int16_t> &alsThresholdsDown, const QList<std::int16_t> &brightnessValues,
                         const QList<std::int16_t> &brightnessValuesDim);
    Result setAlsMode(Sidekick::AlsMode mode, float brightenAlpha, float dimmingAlpha);
    Result beginDisplay(Sidekick::DisplayPowerState state, bool logFailure = true);
    Result endDisplay(Sidekick::DisplayPowerState state);

    Result prepareTwm();
    Result setTwmConfig(std::uint32_t msDisplayTimeout, bool tiltToBright);
    Result enterTwm();

signals:
    void connected();
    void disconnected();

private:
    struct Reply;

    static void onServiceRegistered(gbinder_servicemanager *manager, const char *name, void *self);
    static void onServiceDied(gbinder_remote_object *object, void *self);
    bool attach(gbinder_remote_object *object);
    void detach();

    gbinder_local_request *newRequest(Sidekick::Transaction code) const;
    std::optional<Reply> transact(Sidekick::Transaction code, gbinder_local_request *request, const char *name);
    Result statusCall(Sidekick::Transaction code, gbinder_local_request *request, const char *name,
                      bool logFailure = true);
    Result voidCall(Sidekick::Transaction code, gbinder_local_request *request, const char *name);
    Result resourceCall(Sidekick::Transaction code, gbinder_local_request *request, const char *name);

    QString m_serviceName;
    gbinder_servicemanager *m_manager = nullptr;
    gbinder_remote_object *m_object = nullptr;
    gbinder_client *m_client = nullptr;
    unsigned long m_registrationHandler = 0;
    unsigned long m_deathHandler = 0;
};

}

#endif
