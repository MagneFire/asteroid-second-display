// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "hands.h"

#include <algorithm>

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

bool Hands::isValidPosition(int position) const
{
    return position >= 0 && position < static_cast<int>(resolution());
}

bool Hands::watchMode() const
{
    return supported() && m_backend->watchMode();
}

uint Hands::resolution() const
{
    return m_backend->handResolution();
}

bool Hands::ResumeWatchMode()
{
    return supported() && m_backend->resumeWatchMode();
}

QList<int> Hands::GetPositions()
{
    return supported() ? m_backend->handPositions() : QList<int>();
}

bool Hands::MoveHand(int hand, int position)
{
    return supported() && isValidHand(hand) && isValidPosition(position)
        && m_backend->moveHand(static_cast<Hand>(hand), position);
}

bool Hands::MoveAllHands(const QList<int> &positions)
{
    const bool valid = positions.size() == 2
        && std::all_of(positions.cbegin(), positions.cend(), [this](int position) { return isValidPosition(position); });
    return supported() && valid && m_backend->moveAllHands(positions);
}

bool Hands::Calibrate(int hand, int rotation, int steps)
{
    return supported() && isValidHand(hand) && isValidRotation(rotation) && steps > 0 && isValidPosition(steps)
        && m_backend->calibrateHand(static_cast<Hand>(hand), static_cast<Rotation>(rotation), steps);
}

}
