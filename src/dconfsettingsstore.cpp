// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "dconfsettingsstore.h"

#include <mdconfitem.h>

namespace SecondDisplay {

MDConfItem *DConfSettingsStore::item(const QString &key) const
{
    MDConfItem *&item = m_items[key];
    if (!item)
        item = new MDConfItem(key, const_cast<DConfSettingsStore *>(this));
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
