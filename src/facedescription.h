// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef FACEDESCRIPTION_H
#define FACEDESCRIPTION_H

#include <QPoint>
#include <QString>

namespace SecondDisplay {

struct FaceDescription
{
    QString backgroundPng;
    QString digitsPng;
    QString minuteDigitsPng;
    QString colonPng;
    QPoint hours;
    QPoint minutes;
    QPoint colon;
    bool color = false;
};

}

#endif
