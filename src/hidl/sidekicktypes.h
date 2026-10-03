// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SIDEKICKTYPES_H
#define SIDEKICKTYPES_H

#include <cstddef>
#include <cstdint>

namespace SecondDisplay::Sidekick {

inline constexpr char Interface10[] = "vendor.google_clockwork.sidekickgraphics@1.0::ISidekickGraphics";
inline constexpr char Interface11[] = "vendor.google_clockwork.sidekickgraphics@1.1::ISidekickGraphics";
inline constexpr char Interface12[] = "vendor.google_clockwork.sidekickgraphics@1.2::ISidekickGraphics";
inline constexpr char DefaultService[] = "vendor.google_clockwork.sidekickgraphics@1.2::ISidekickGraphics/default";

enum class Transaction : std::uint32_t {
    Reset = 1,
    GetCapabilities = 2,
    SetAlsMode = 3,
    SetBrightness = 4,
    GetLastBrightness = 5,
    SetDesiredFps = 6,
    BeginResources = 7,
    EndResources = 8,
    DeleteResources = 9,
    GetBytesAvailable = 10,
    BeginDisplay = 11,
    EndDisplay = 12,
    BeginProfiling = 13,
    EndProfiling = 14,
    ReadFramebuffer = 15,
    SendBitmapPng8888 = 16,
    SendFontPng8888 = 17,
    SendNumberResource = 18,
    ReplaceDrawableInfo = 19,
    UpdateDisplayTime = 20,
    EnterTwm = 21,
    SetTwmConfig = 22,
    SendProportionalFontPng8888 = 23,
    SendStringResource = 24,
    GetColorFormat = 25,
    SetColorFormat = 26,
    SendCustomFont = 27,
    SendColorNumberResource = 28,
    SendColorStringResource = 29,
    SendDateTimeResource = 30,
    PrepareTwm = 31,
};

inline constexpr std::uint32_t LastCode10 = 22;
inline constexpr std::uint32_t LastCode11 = 24;
inline constexpr std::uint32_t LastCode12 = 31;

enum class Status : std::uint32_t {
    Ok = 0,
    UnknownError = 1,
    BadValue = 2,
    UnsupportedOperation = 3,
    InsufficientResource = 4,
};

enum class DisplayPowerState : std::uint32_t {
    Full = 0,
    Idle = 1,
    Off = 2,
    Unknown = 3,
};

enum class DrawableType : std::uint32_t {
    Background = 0,
    Generic = 1,
    Number = 2,
    Rotating = 3,
    String = 4,
    DateTime = 5,
};

enum class ColorFormat : std::uint32_t {
    Rgb332 = 0,
    Rgb565 = 1,
    Gray = 2,
};

enum class AlsMode : std::uint32_t {
    Off = 0,
    Read = 1,
    On = 2,
};

namespace Capability {
enum : std::uint32_t {
    Bitmap = 1u << 0,
    RotateBitmap = 1u << 1,
    ScaleBitmap = 1u << 2,
    FlipBitmap = 1u << 3,
    ReadFramebuffer = 1u << 8,
    ReadAls = 1u << 16,
    ControlAls = 1u << 17,
    Brightness = 1u << 19,
    Number = 1u << 24,
};
}

struct Time
{
    std::int32_t daysSinceLocalEpoch = 0;
    std::int32_t msSinceMidnight = 0;
};
static_assert(sizeof(Time) == 8);

struct RotationInfo
{
    bool hasRotation = false;
    float pivotX = 0;
    float pivotY = 0;
    float degreesPerDay = 0;
    float degreesPerStep = 0;
    Time zeroDegreesTime;
};
static_assert(sizeof(RotationInfo) == 28);

struct TransformInfo
{
    bool transformed = false;
    bool flipX = false;
    bool flipY = false;
    bool flip45 = false;
    float scaleX = 0;
    float scaleY = 0;
};
static_assert(sizeof(TransformInfo) == 12);

struct BlinkInfo
{
    bool blinking = false;
    float periodOnMs = 0;
    float periodOffMs = 0;
    Time startTime;
};
static_assert(sizeof(BlinkInfo) == 20);

struct DrawableInfo
{
    std::int32_t id = 0;
    std::int32_t width = 0;
    std::int32_t height = 0;
    bool display = true;
    float offsetX = 0;
    float offsetY = 0;
    RotationInfo rotationInfo;
    TransformInfo transformInfo;
    DrawableType type = DrawableType::Generic;
    BlinkInfo blink;
    std::int32_t zOrder = 0;
    bool displayInTwm = false;
};
static_assert(sizeof(DrawableInfo) == 96);
static_assert(offsetof(DrawableInfo, rotationInfo) == 24);
static_assert(offsetof(DrawableInfo, transformInfo) == 52);
static_assert(offsetof(DrawableInfo, type) == 64);
static_assert(offsetof(DrawableInfo, blink) == 68);
static_assert(offsetof(DrawableInfo, zOrder) == 88);
static_assert(offsetof(DrawableInfo, displayInTwm) == 92);

struct FontInfo
{
    std::int32_t width = 0;
    std::int32_t height = 0;
    std::int32_t nGlyphs = 0;
    std::int32_t id = 0;
};
static_assert(sizeof(FontInfo) == 16);

struct NumberInfo
{
    Time startTime;
    std::int32_t startNumber = 0;
    std::int32_t minNumber = 0;
    std::int32_t maxNumber = 0;
    std::int32_t increment = 1;
    float msPerIncrement = 0;
    std::int32_t fontId = 0;
    std::int32_t digits = 0;
    std::int32_t leadingZeroes = 0;
    std::int32_t digit = -1;
    float transitionMs = 0;
};
static_assert(sizeof(NumberInfo) == 48);

struct ColorCapability
{
    std::int32_t redBits = 0;
    std::int32_t greenBits = 0;
    std::int32_t blueBits = 0;
    std::int32_t paletteSize = 0;
    bool oneBitAlpha = false;
    bool multiBitAlpha = false;
};
static_assert(sizeof(ColorCapability) == 20);

}

#endif
