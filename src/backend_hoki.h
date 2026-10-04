// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef BACKEND_HOKI_H
#define BACKEND_HOKI_H

#include <optional>

#include "backend.h"
#include "sidekickclient.h"
#include "face.h"

namespace SecondDisplay {

class HokiBackend : public Backend
{
    Q_OBJECT

public:
    explicit HokiBackend(const QString &faceDirectory = QString::fromLatin1(HOKI_FACE_DIR), QObject *parent = nullptr);

    unsigned int capabilities() const override;
    bool synchronizeTime(TimeFormat format) override;
    bool prepareTimepiece() override;
    bool enterTimepiece() override;
    bool setAodOffloadEnabled(bool enabled) override;
    void setAmbientEnabled(bool enabled) override;
    void displayStateChanged(const QString &state) override;
    bool aodOffloadActive() const override;
    bool releaseAodOffload() override;
    bool setFace(const FaceDescription &description) override;
    void clearFace() override;

private:
    void onConnected();
    void onDisconnected();
    std::optional<Face> buildFace() const;
    bool uploadFace(const Face &face);
    bool ensureFaceLoaded();
    Sidekick::ColorFormat colorFormat() const;
    void reloadFace();
    bool configureTimepiece();
    bool blankDisplay() const;
    void startOffload();
    void stopOffload();
    void setOffloadActive(bool active);

    QString m_faceDirectory;
    std::optional<FaceDescription> m_face;
    SidekickClient m_client;
    std::optional<SidekickClient::Capabilities> m_capabilities;
    TimeFormat m_format = TimeFormat::TwentyFourHour;
    bool m_timepiecePrepared = false;
    bool m_faceLoaded = false;
    bool m_offloadEnabled = false;
    bool m_ambientEnabled = true;
    bool m_displayOff = false;
    bool m_offloadActive = false;
};

}

#endif
