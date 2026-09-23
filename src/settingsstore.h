// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SETTINGSSTORE_H
#define SETTINGSSTORE_H

#include <QHash>
#include <QObject>
#include <QVariant>
#include <memory>

namespace SecondDisplay {

class SettingsStore : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

    virtual QVariant value(const QString &key) const = 0;
    virtual void setValue(const QString &key, const QVariant &value) = 0;

signals:
    void valueChanged(const QString &key);
};

class MemorySettingsStore : public SettingsStore
{
    Q_OBJECT

public:
    using SettingsStore::SettingsStore;

    QVariant value(const QString &key) const override;
    void setValue(const QString &key, const QVariant &value) override;

private:
    QHash<QString, QVariant> m_values;
};

std::unique_ptr<SettingsStore> createSettingsStore();

}

#endif
