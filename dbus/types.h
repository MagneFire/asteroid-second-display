// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SECONDDISPLAY_TYPES_H
#define SECONDDISPLAY_TYPES_H

namespace SecondDisplay {

inline constexpr char ServiceName[] = "org.asteroid.SecondDisplay";
inline constexpr char DisplayPath[] = "/Display";
inline constexpr char HandsPath[] = "/Hands";
inline constexpr char DisplayInterface[] = "org.asteroid.SecondDisplay.Display";
inline constexpr char HandsInterface[] = "org.asteroid.SecondDisplay.Hands";

namespace Capability {
enum : unsigned int {
    TimeSync = 1u << 0,
    TimepieceMode = 1u << 1,
    StepCounter = 1u << 2,
    Hands = 1u << 6,
};
}

}

#endif
