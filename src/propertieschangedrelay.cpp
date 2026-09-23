// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "propertieschangedrelay.h"

#include <QDBusMessage>
#include <QMetaProperty>

namespace SecondDisplay {

PropertiesChangedRelay::PropertiesChangedRelay(QObject *target, const QString &path, const QString &interface,
                                               const QDBusConnection &connection)
    : QObject(target)
    , m_target(target)
    , m_path(path)
    , m_interface(interface)
    , m_connection(connection)
{
    const QMetaObject *meta = target->metaObject();
    const QMetaMethod relay = metaObject()->method(metaObject()->indexOfSlot("onPropertyChanged()"));
    for (int i = meta->propertyOffset(); i < meta->propertyCount(); ++i) {
        const QMetaProperty property = meta->property(i);
        if (!property.hasNotifySignal())
            continue;
        m_propertyBySignal.insert(property.notifySignalIndex(), property.name());
        connect(target, property.notifySignal(), this, relay);
    }
}

void PropertiesChangedRelay::onPropertyChanged()
{
    const QByteArray name = m_propertyBySignal.value(senderSignalIndex());
    QDBusMessage signal = QDBusMessage::createSignal(m_path, "org.freedesktop.DBus.Properties", "PropertiesChanged");
    signal << m_interface << QVariantMap{{QString::fromLatin1(name), m_target->property(name)}} << QStringList();
    m_connection.send(signal);
}

}
