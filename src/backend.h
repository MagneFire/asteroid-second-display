// SPDX-FileCopyrightText: 2023-2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <memory>

namespace SecondDisplay {

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

    virtual bool HasSecondDisplay() { return false; };
    virtual int SynchronizeTime() { return 0; };
    virtual bool HasTimepieceMode() { return false; };
    virtual int EnterTimepieceMode() { return 0; };
    virtual bool HasStepCounter() { return false; };
    virtual int SetStepCounter(bool enable) { return 0; };
    virtual int HasHands() { return false; };
    virtual Hands* GetHands() { return &hands; };

private:
    Hands hands;
};

std::unique_ptr<Backend> createBackend(const QString &machine);

}

#endif
