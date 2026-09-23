// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "settingsstore.h"

#ifdef HAVE_MLITE
#include "dconfsettingsstore.h"
#endif

namespace SecondDisplay {

QVariant MemorySettingsStore::value(const QString &key) const
{
    return m_values.value(key);
}

void MemorySettingsStore::setValue(const QString &key, const QVariant &value)
{
    m_values.insert(key, value);
}

std::unique_ptr<SettingsStore> createSettingsStore()
{
#ifdef HAVE_MLITE
    return std::make_unique<DConfSettingsStore>();
#else
    return std::make_unique<MemorySettingsStore>();
#endif
}

}
