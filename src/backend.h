// SPDX-FileCopyrightText: 2023-2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef BACKEND_H
#define BACKEND_H

#include <QList>
#include <QObject>
#include <memory>

#include "facedescription.h"
#include "types.h"

namespace SecondDisplay {

enum class TimeFormat { TwentyFourHour, TwelveHour };

class Backend : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

    virtual unsigned int capabilities() const { return 0; }

    virtual bool synchronizeTime(TimeFormat) { return false; }
    virtual bool prepareTimepiece() { return false; }
    virtual bool enterTimepiece() { return true; }

    virtual bool setStepCounterEnabled(bool) { return false; }
    virtual bool setHeartRateEnabled(bool) { return false; }
    virtual bool setMotionEnabled(bool) { return false; }
    virtual bool setBackground(Background) { return false; }
    virtual bool setAodOffloadEnabled(bool) { return false; }
    virtual void setAmbientEnabled(bool) {}
    virtual void displayStateChanged(const QString &) {}
    virtual bool aodOffloadActive() const { return false; }
    virtual bool releaseAodOffload() { return true; }
    virtual bool setFace(const FaceDescription &) { return false; }
    virtual void clearFace() {}

    virtual bool watchMode() const { return false; }
    virtual unsigned int handResolution() const { return 0; }
    virtual bool resumeWatchMode() { return false; }
    virtual QList<int> handPositions() const { return {}; }
    virtual bool moveHand(Hand, int) { return false; }
    virtual bool moveAllHands(const QList<int> &) { return false; }
    virtual bool calibrateHand(Hand, Rotation, int) { return false; }

signals:
    void capabilitiesChanged();
    void watchModeChanged();
    void aodOffloadActiveChanged();
};

std::unique_ptr<Backend> createBackend(const QString &machine);

}

#endif
