// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef PROPERTIESCHANGEDRELAY_H
#define PROPERTIESCHANGEDRELAY_H

#include <QDBusConnection>
#include <QHash>
#include <QObject>

namespace SecondDisplay {

class PropertiesChangedRelay : public QObject
{
    Q_OBJECT

public:
    PropertiesChangedRelay(QObject *target, const QString &path, const QString &interface,
                           const QDBusConnection &connection);

private slots:
    void onPropertyChanged();

private:
    QObject *m_target;
    QString m_path;
    QString m_interface;
    QDBusConnection m_connection;
    QHash<int, QByteArray> m_propertyBySignal;
};

}

#endif
