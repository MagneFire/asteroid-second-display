// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DCONFSETTINGSSTORE_H
#define DCONFSETTINGSSTORE_H

#include <QHash>

#include "settingsstore.h"

class MDConfItem;

namespace SecondDisplay {

class DConfSettingsStore : public SettingsStore
{
    Q_OBJECT

public:
    using SettingsStore::SettingsStore;

    QVariant value(const QString &key) const override;
    void setValue(const QString &key, const QVariant &value) override;

private:
    MDConfItem *item(const QString &key) const;

    mutable QHash<QString, MDConfItem *> m_items;
};

}

#endif
