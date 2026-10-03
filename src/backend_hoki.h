// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef BACKEND_HOKI_H
#define BACKEND_HOKI_H

#include <QTimer>
#include <optional>

#include "backend.h"
#include "sidekickclient.h"
#include "timepieceface.h"

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

private:
    void connectToService();
    void onConnected();
    void onDisconnected();
    bool uploadFace(const TimepieceFace &face);
    bool configureTimepiece();
    bool blankDisplay() const;

    QString m_faceDirectory;
    SidekickClient m_client;
    QTimer m_retryTimer;
    std::optional<SidekickClient::Capabilities> m_capabilities;
    TimeFormat m_format = TimeFormat::TwentyFourHour;
    bool m_timepiecePrepared = false;
};

}

#endif
