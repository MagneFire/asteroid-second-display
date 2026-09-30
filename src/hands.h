// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef HANDS_H
#define HANDS_H

#include <QList>
#include <QObject>

#include "backend.h"

namespace SecondDisplay {

class Hands : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool WatchMode READ watchMode NOTIFY watchModeChanged)
    Q_PROPERTY(uint Resolution READ resolution CONSTANT)

public:
    explicit Hands(Backend *backend, QObject *parent = nullptr);

    bool watchMode() const;
    uint resolution() const;

public slots:
    bool ResumeWatchMode();
    QList<int> GetPositions();
    bool MoveHand(int hand, int position);
    bool MoveAllHands(const QList<int> &positions);
    bool Calibrate(int hand, int rotation, int steps);

signals:
    void watchModeChanged();

private:
    bool supported() const;
    bool isValidPosition(int position) const;

    Backend *m_backend;
};

}

#endif
