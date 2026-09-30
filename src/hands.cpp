// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "hands.h"

namespace SecondDisplay {

namespace {

bool isValidHand(int hand)
{
    return hand == static_cast<int>(Hand::Minute) || hand == static_cast<int>(Hand::Hour);
}

bool isValidRotation(int rotation)
{
    return rotation == static_cast<int>(Rotation::Clockwise) || rotation == static_cast<int>(Rotation::CounterClockwise);
}

}

Hands::Hands(Backend *backend, QObject *parent)
    : QObject(parent)
    , m_backend(backend)
{
    connect(m_backend, &Backend::watchModeChanged, this, &Hands::watchModeChanged);
}

bool Hands::supported() const
{
    return m_backend->capabilities() & Capability::Hands;
}

bool Hands::watchMode() const
{
    return supported() && m_backend->watchMode();
}

bool Hands::ResumeWatchMode()
{
    return supported() && m_backend->resumeWatchMode();
}

bool Hands::MoveHand(int hand, int position)
{
    return supported() && isValidHand(hand) && m_backend->moveHand(static_cast<Hand>(hand), position);
}

bool Hands::MoveAllHands(const QList<int> &positions)
{
    return supported() && positions.size() == 2 && m_backend->moveAllHands(positions);
}

bool Hands::Calibrate(int hand, int rotation, int steps)
{
    return supported() && isValidHand(hand) && isValidRotation(rotation) && steps > 0
        && m_backend->calibrateHand(static_cast<Hand>(hand), static_cast<Rotation>(rotation), steps);
}

}
