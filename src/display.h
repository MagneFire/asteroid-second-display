// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DISPLAY_H
#define DISPLAY_H

#include <QObject>

#include "backend.h"

namespace SecondDisplay {

class Display : public QObject
{
    Q_OBJECT
    Q_PROPERTY(uint Capabilities READ capabilities NOTIFY capabilitiesChanged)
    Q_PROPERTY(bool StepCounterEnabled READ stepCounterEnabled WRITE setStepCounterEnabled NOTIFY stepCounterEnabledChanged)

public:
    explicit Display(Backend *backend, QObject *parent = nullptr);

    uint capabilities() const;

    bool stepCounterEnabled() const;
    void setStepCounterEnabled(bool enabled);

public slots:
    bool SynchronizeTime();
    bool EnterTimepieceMode(bool powerOff);

signals:
    void capabilitiesChanged();
    void stepCounterEnabledChanged();

private:
    bool supports(unsigned int capability) const;
    bool applyFeature(bool &currentlyEnabled, bool enabled, unsigned int capability, bool (Backend::*setEnabled)(bool));

    Backend *m_backend;
    bool m_stepCounterEnabled = false;
};

}

#endif
