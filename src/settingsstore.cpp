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
    if (m_values.value(key) == value)
        return;
    m_values.insert(key, value);
    emit valueChanged(key);
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
