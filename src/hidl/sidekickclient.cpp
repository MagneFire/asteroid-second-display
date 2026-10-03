// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "sidekickclient.h"

#include <QDebug>
#include <gbinder.h>
#include <utility>

namespace SecondDisplay {

using namespace Sidekick;

namespace {

constexpr char HwBinderDevice[] = "/dev/hwbinder";

const GBinderClientIfaceInfo Interfaces[] = {
    {Interface10, LastCode10},
    {Interface11, LastCode11},
    {Interface12, LastCode12},
};

const char *statusName(Status status)
{
    switch (status) {
    case Status::Ok:
        return "OK";
    case Status::UnknownError:
        return "UNKNOWN_ERROR";
    case Status::BadValue:
        return "BAD_VALUE";
    case Status::UnsupportedOperation:
        return "UNSUPPORTED_OPERATION";
    case Status::InsufficientResource:
        return "INSUFFICIENT_RESOURCE";
    }
    return "?";
}

template<typename T>
void appendStruct(GBinderWriter *writer, const T &value)
{
    gbinder_writer_append_buffer_object(writer, gbinder_writer_memdup(writer, &value, sizeof value), sizeof value);
}

void appendByteVec(GBinderWriter *writer, const QByteArray &bytes)
{
    const void *data = gbinder_writer_memdup(writer, bytes.constData(), bytes.size());
    gbinder_writer_append_hidl_vec(writer, data, bytes.size(), 1);
}

template<typename T>
void appendVec(GBinderWriter *writer, const QList<T> &values)
{
    const void *data = values.isEmpty() ? nullptr
                                        : gbinder_writer_memdup(writer, values.constData(), values.size() * sizeof(T));
    gbinder_writer_append_hidl_vec(writer, data, values.size(), sizeof(T));
}

}

struct SidekickClient::Reply
{
    GBinderRemoteReply *reply = nullptr;
    GBinderReader reader{};

    Reply() = default;
    Reply(Reply &&other) noexcept
        : reply(std::exchange(other.reply, nullptr))
        , reader(other.reader)
    {
    }
    Reply(const Reply &) = delete;
    Reply &operator=(const Reply &) = delete;
    Reply &operator=(Reply &&) = delete;
    ~Reply() { gbinder_remote_reply_unref(reply); }

    bool readUInt32(std::uint32_t *value) { return gbinder_reader_read_uint32(&reader, value); }
    bool readStatus(Status *status)
    {
        std::uint32_t value = 0;
        if (!readUInt32(&value))
            return false;
        *status = static_cast<Status>(value);
        return true;
    }
};

bool SidekickClient::Result::ok() const
{
    return binderStatus == GBINDER_STATUS_OK && status == Status::Ok;
}

QString SidekickClient::Result::toString() const
{
    if (binderStatus != GBINDER_STATUS_OK)
        return QStringLiteral("binder error %1").arg(binderStatus);
    return QString::fromLatin1(statusName(status));
}

SidekickClient::SidekickClient(const QString &serviceName, QObject *parent)
    : QObject(parent)
    , m_serviceName(serviceName)
{
}

SidekickClient::~SidekickClient()
{
    detach();
    if (m_manager) {
        if (m_registrationHandler)
            gbinder_servicemanager_remove_handler(m_manager, m_registrationHandler);
        gbinder_servicemanager_unref(m_manager);
    }
}

QString SidekickClient::serviceName() const
{
    return m_serviceName;
}

bool SidekickClient::isConnected() const
{
    return m_client != nullptr;
}

QStringList SidekickClient::listServices()
{
    QStringList names;
    GBinderServiceManager *manager = gbinder_servicemanager_new2(HwBinderDevice, "hidl", "hidl");
    if (!manager)
        return names;
    char **services = gbinder_servicemanager_list_sync(manager);
    for (char **name = services; name && *name; ++name)
        names.append(QString::fromUtf8(*name));
    g_strfreev(services);
    gbinder_servicemanager_unref(manager);
    return names;
}

bool SidekickClient::connectToService()
{
    if (m_client)
        return true;
    if (!m_manager) {
        m_manager = gbinder_servicemanager_new2(HwBinderDevice, "hidl", "hidl");
        if (!m_manager) {
            qWarning() << "Unable to open" << HwBinderDevice;
            return false;
        }
    }
    const QByteArray name = m_serviceName.toLatin1();
    int status = 0;
    GBinderRemoteObject *object = gbinder_servicemanager_get_service_sync(m_manager, name.constData(), &status);
    if (!object) {
        if (!m_registrationHandler)
            qInfo() << m_serviceName << "is not registered (status" << status << "), waiting for it";
        if (!m_registrationHandler) {
            m_registrationHandler = gbinder_servicemanager_add_registration_handler(m_manager, name.constData(),
                                                                                    onServiceRegistered, this);
        }
        return false;
    }
    return attach(object);
}

bool SidekickClient::attach(GBinderRemoteObject *object)
{
    m_object = gbinder_remote_object_ref(object);
    m_client = gbinder_client_new2(m_object, Interfaces, G_N_ELEMENTS(Interfaces));
    if (!m_client) {
        qWarning() << "Unable to create a client for" << m_serviceName;
        gbinder_remote_object_unref(m_object);
        m_object = nullptr;
        return false;
    }
    m_deathHandler = gbinder_remote_object_add_death_handler(m_object, onServiceDied, this);
    if (m_registrationHandler) {
        gbinder_servicemanager_remove_handler(m_manager, m_registrationHandler);
        m_registrationHandler = 0;
    }
    qInfo() << "Connected to" << m_serviceName;
    emit connected();
    return true;
}

void SidekickClient::detach()
{
    if (m_object && m_deathHandler) {
        gbinder_remote_object_remove_handler(m_object, m_deathHandler);
        m_deathHandler = 0;
    }
    if (m_client) {
        gbinder_client_unref(m_client);
        m_client = nullptr;
    }
    if (m_object) {
        gbinder_remote_object_unref(m_object);
        m_object = nullptr;
    }
}

void SidekickClient::onServiceRegistered(GBinderServiceManager *, const char *name, void *self)
{
    auto *client = static_cast<SidekickClient *>(self);
    if (client->m_serviceName != QString::fromLatin1(name))
        return;
    client->connectToService();
}

void SidekickClient::onServiceDied(GBinderRemoteObject *, void *self)
{
    auto *client = static_cast<SidekickClient *>(self);
    qWarning() << client->m_serviceName << "died";
    client->detach();
    emit client->disconnected();
    client->connectToService();
}

GBinderLocalRequest *SidekickClient::newRequest(Transaction code) const
{
    return gbinder_client_new_request2(m_client, static_cast<guint32>(code));
}

std::optional<SidekickClient::Reply> SidekickClient::transact(Transaction code, GBinderLocalRequest *request,
                                                              const char *name)
{
    if (!m_client) {
        qWarning() << name << "called without a connection";
        gbinder_local_request_unref(request);
        return std::nullopt;
    }
    Reply reply;
    int status = GBINDER_STATUS_OK;
    reply.reply = gbinder_client_transact_sync_reply(m_client, static_cast<guint32>(code), request, &status);
    gbinder_local_request_unref(request);
    if (status != GBINDER_STATUS_OK || !reply.reply) {
        qWarning() << name << "failed, binder status" << status;
        return std::nullopt;
    }
    gbinder_remote_reply_init_reader(reply.reply, &reply.reader);
    std::int32_t hidlStatus = 0;
    if (!gbinder_reader_read_int32(&reply.reader, &hidlStatus) || hidlStatus != 0) {
        qWarning() << name << "returned HIDL error" << hidlStatus;
        return std::nullopt;
    }
    return reply;
}

SidekickClient::Result SidekickClient::statusCall(Transaction code, GBinderLocalRequest *request, const char *name)
{
    Result result;
    auto reply = transact(code, request, name);
    if (!reply)
        return result;
    result.binderStatus = GBINDER_STATUS_OK;
    if (!reply->readStatus(&result.status))
        result.status = Status::UnknownError;
    if (result.status != Status::Ok)
        qWarning() << name << "returned" << statusName(result.status);
    return result;
}

SidekickClient::Result SidekickClient::voidCall(Transaction code, GBinderLocalRequest *request, const char *name)
{
    Result result;
    if (transact(code, request, name)) {
        result.binderStatus = GBINDER_STATUS_OK;
        result.status = Status::Ok;
    }
    return result;
}

SidekickClient::Result SidekickClient::resourceCall(Transaction code, GBinderLocalRequest *request, const char *name)
{
    Result result;
    auto reply = transact(code, request, name);
    if (!reply)
        return result;
    result.binderStatus = GBINDER_STATUS_OK;
    std::uint32_t bytesUsed = 0;
    if (!reply->readStatus(&result.status) || !reply->readUInt32(&bytesUsed))
        result.status = Status::UnknownError;
    if (result.status != Status::Ok)
        qWarning() << name << "returned" << statusName(result.status);
    else
        qDebug() << name << "used" << bytesUsed << "bytes";
    return result;
}

std::optional<SidekickClient::Capabilities> SidekickClient::getCapabilities()
{
    if (!m_client)
        return std::nullopt;
    auto reply = transact(Transaction::GetCapabilities, newRequest(Transaction::GetCapabilities), "getCapabilities");
    if (!reply)
        return std::nullopt;
    Capabilities capabilities;
    Status status = Status::UnknownError;
    if (!reply->readStatus(&status) || !reply->readUInt32(&capabilities.capabilities))
        return std::nullopt;
    const auto *color = gbinder_reader_read_hidl_struct(&reply->reader, ColorCapability);
    if (!color || !reply->readUInt32(&capabilities.bytesAvailable) || !reply->readUInt32(&capabilities.displayWidth)
        || !reply->readUInt32(&capabilities.displayHeight))
        return std::nullopt;
    capabilities.color = *color;
    if (status != Status::Ok) {
        qWarning() << "getCapabilities returned" << statusName(status);
        return std::nullopt;
    }
    return capabilities;
}

std::optional<std::uint32_t> SidekickClient::getBytesAvailable()
{
    if (!m_client)
        return std::nullopt;
    auto reply = transact(Transaction::GetBytesAvailable, newRequest(Transaction::GetBytesAvailable),
                          "getBytesAvailable");
    std::uint32_t bytes = 0;
    if (!reply || !reply->readUInt32(&bytes))
        return std::nullopt;
    return bytes;
}

std::optional<ColorFormat> SidekickClient::getColorFormat()
{
    if (!m_client)
        return std::nullopt;
    auto reply = transact(Transaction::GetColorFormat, newRequest(Transaction::GetColorFormat), "getColorFormat");
    Status status = Status::UnknownError;
    std::uint32_t format = 0;
    if (!reply || !reply->readStatus(&status) || !reply->readUInt32(&format) || status != Status::Ok)
        return std::nullopt;
    return static_cast<ColorFormat>(format);
}

SidekickClient::Result SidekickClient::setColorFormat(ColorFormat format)
{
    if (!m_client)
        return {};
    GBinderLocalRequest *request = newRequest(Transaction::SetColorFormat);
    gbinder_local_request_append_int32(request, static_cast<guint32>(format));
    return statusCall(Transaction::SetColorFormat, request, "setColorFormat");
}

std::optional<QByteArray> SidekickClient::readFramebuffer()
{
    if (!m_client)
        return std::nullopt;
    auto reply = transact(Transaction::ReadFramebuffer, newRequest(Transaction::ReadFramebuffer), "readFramebuffer");
    Status status = Status::UnknownError;
    if (!reply || !reply->readStatus(&status))
        return std::nullopt;
    gsize count = 0;
    const auto *data = gbinder_reader_read_hidl_byte_vec(&reply->reader, &count);
    if (!data || status != Status::Ok) {
        qWarning() << "readFramebuffer returned" << statusName(status);
        return std::nullopt;
    }
    return QByteArray(reinterpret_cast<const char *>(data), count);
}

SidekickClient::Result SidekickClient::reset()
{
    if (!m_client)
        return {};
    return statusCall(Transaction::Reset, newRequest(Transaction::Reset), "reset");
}

SidekickClient::Result SidekickClient::beginResources()
{
    if (!m_client)
        return {};
    return statusCall(Transaction::BeginResources, newRequest(Transaction::BeginResources), "beginResources");
}

SidekickClient::Result SidekickClient::endResources()
{
    if (!m_client)
        return {};
    return statusCall(Transaction::EndResources, newRequest(Transaction::EndResources), "endResources");
}

SidekickClient::Result SidekickClient::deleteResources(const QList<std::uint32_t> &ids)
{
    if (!m_client)
        return {};
    GBinderLocalRequest *request = newRequest(Transaction::DeleteResources);
    GBinderWriter writer;
    gbinder_local_request_init_writer(request, &writer);
    appendVec(&writer, ids);
    return statusCall(Transaction::DeleteResources, request, "deleteResources");
}

SidekickClient::Result SidekickClient::sendBitmapPng8888(const DrawableInfo &drawable, const QByteArray &png)
{
    if (!m_client)
        return {};
    GBinderLocalRequest *request = newRequest(Transaction::SendBitmapPng8888);
    GBinderWriter writer;
    gbinder_local_request_init_writer(request, &writer);
    appendStruct(&writer, drawable);
    appendByteVec(&writer, png);
    return resourceCall(Transaction::SendBitmapPng8888, request, "sendBitmapPng8888");
}

SidekickClient::Result SidekickClient::sendFontPng8888(const FontInfo &font, const QByteArray &png)
{
    if (!m_client)
        return {};
    GBinderLocalRequest *request = newRequest(Transaction::SendFontPng8888);
    GBinderWriter writer;
    gbinder_local_request_init_writer(request, &writer);
    appendStruct(&writer, font);
    appendByteVec(&writer, png);
    return resourceCall(Transaction::SendFontPng8888, request, "sendFontPng8888");
}

SidekickClient::Result SidekickClient::sendNumberResource(const DrawableInfo &drawable, const NumberInfo &number)
{
    if (!m_client)
        return {};
    GBinderLocalRequest *request = newRequest(Transaction::SendNumberResource);
    GBinderWriter writer;
    gbinder_local_request_init_writer(request, &writer);
    appendStruct(&writer, drawable);
    appendStruct(&writer, number);
    return resourceCall(Transaction::SendNumberResource, request, "sendNumberResource");
}

SidekickClient::Result SidekickClient::updateDisplayTime()
{
    if (!m_client)
        return {};
    return voidCall(Transaction::UpdateDisplayTime, newRequest(Transaction::UpdateDisplayTime), "updateDisplayTime");
}

SidekickClient::Result SidekickClient::setBrightness(bool discrete, const QList<std::int16_t> &alsThresholdsUp,
                                                     const QList<std::int16_t> &alsThresholdsDown,
                                                     const QList<std::int16_t> &brightnessValues,
                                                     const QList<std::int16_t> &brightnessValuesDim)
{
    if (!m_client)
        return {};
    GBinderLocalRequest *request = newRequest(Transaction::SetBrightness);
    GBinderWriter writer;
    gbinder_local_request_init_writer(request, &writer);
    gbinder_writer_append_bool(&writer, discrete);
    appendVec(&writer, alsThresholdsUp);
    appendVec(&writer, alsThresholdsDown);
    appendVec(&writer, brightnessValues);
    appendVec(&writer, brightnessValuesDim);
    return statusCall(Transaction::SetBrightness, request, "setBrightness");
}

SidekickClient::Result SidekickClient::setAlsMode(AlsMode mode, float brightenAlpha, float dimmingAlpha)
{
    if (!m_client)
        return {};
    GBinderLocalRequest *request = newRequest(Transaction::SetAlsMode);
    GBinderWriter writer;
    gbinder_local_request_init_writer(request, &writer);
    gbinder_writer_append_int32(&writer, static_cast<guint32>(mode));
    gbinder_writer_append_float(&writer, brightenAlpha);
    gbinder_writer_append_float(&writer, dimmingAlpha);
    return statusCall(Transaction::SetAlsMode, request, "setAlsMode");
}

SidekickClient::Result SidekickClient::beginDisplay(DisplayPowerState state)
{
    if (!m_client)
        return {};
    GBinderLocalRequest *request = newRequest(Transaction::BeginDisplay);
    gbinder_local_request_append_int32(request, static_cast<guint32>(state));
    return statusCall(Transaction::BeginDisplay, request, "beginDisplay");
}

SidekickClient::Result SidekickClient::endDisplay(DisplayPowerState state)
{
    if (!m_client)
        return {};
    GBinderLocalRequest *request = newRequest(Transaction::EndDisplay);
    gbinder_local_request_append_int32(request, static_cast<guint32>(state));
    return voidCall(Transaction::EndDisplay, request, "endDisplay");
}

SidekickClient::Result SidekickClient::prepareTwm()
{
    if (!m_client)
        return {};
    return statusCall(Transaction::PrepareTwm, newRequest(Transaction::PrepareTwm), "prepareTWM");
}

SidekickClient::Result SidekickClient::setTwmConfig(std::uint32_t msDisplayTimeout, bool tiltToBright)
{
    if (!m_client)
        return {};
    GBinderLocalRequest *request = newRequest(Transaction::SetTwmConfig);
    GBinderWriter writer;
    gbinder_local_request_init_writer(request, &writer);
    gbinder_writer_append_int32(&writer, msDisplayTimeout);
    gbinder_writer_append_bool(&writer, tiltToBright);
    return statusCall(Transaction::SetTwmConfig, request, "setTwmConfig");
}

SidekickClient::Result SidekickClient::enterTwm()
{
    if (!m_client)
        return {};
    return statusCall(Transaction::EnterTwm, newRequest(Transaction::EnterTwm), "enterTwm");
}

}
