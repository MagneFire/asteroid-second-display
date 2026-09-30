// SPDX-FileCopyrightText: 2023-2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <memory>

#include "types.h"

namespace SecondDisplay {

enum class TimeFormat { TwentyFourHour, TwelveHour };

class Hands : public QObject
{
   public:
    Hands(){};
    virtual int HasHands() { return false; };
    virtual int SetWatchMode(bool enable) { return false; };
    virtual int MoveHands(int, int) { return false; };
    virtual int MoveAllHands(int) { return false; };
    virtual int Calibrate(int, int) { return false; };
};

class Backend : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

    virtual unsigned int capabilities() const { return 0; }

    virtual bool synchronizeTime(TimeFormat) { return false; }
    virtual bool prepareTimepiece() { return false; }

    virtual bool setStepCounterEnabled(bool) { return false; }
    virtual bool setHeartRateEnabled(bool) { return false; }
    virtual bool setMotionEnabled(bool) { return false; }
    virtual bool setBackground(Background) { return false; }

    virtual Hands* GetHands() { return &hands; };

signals:
    void capabilitiesChanged();

private:
    Hands hands;
};

std::unique_ptr<Backend> createBackend(const QString &machine);

}

#endif
