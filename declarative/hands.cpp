#include "hands.h"

#include "hands_interface.h"

#include "dbus.h"

Hands::Hands(QObject *parent) : QObject(parent)
{
    hands = new org::asteroid::hands(SERVICE_NAME, HANDS_OBJECT, QDBusConnection::sessionBus(), this);
}

Hands::~Hands()
{
}

bool Hands::hasHands() const
{
    if (!hands->isValid()) {
        qDebug() << "No remote connection! Daemon not active?";
        return false;
    }

    return hands->HasHands();
}