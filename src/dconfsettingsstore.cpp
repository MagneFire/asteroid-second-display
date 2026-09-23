// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "dconfsettingsstore.h"

#include <mdconfitem.h>

namespace SecondDisplay {

MDConfItem *DConfSettingsStore::item(const QString &key) const
{
    MDConfItem *&item = m_items[key];
    if (!item) {
        auto *store = const_cast<DConfSettingsStore *>(this);
        item = new MDConfItem(key, store);
        connect(item, &MDConfItem::valueChanged, store, [store, key] { emit store->valueChanged(key); });
    }
    return item;
}

QVariant DConfSettingsStore::value(const QString &key) const
{
    return item(key)->value();
}

void DConfSettingsStore::setValue(const QString &key, const QVariant &value)
{
    item(key)->set(value);
}

}
