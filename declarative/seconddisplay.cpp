#include "seconddisplay.h"

#include "display_interface.h"

#include "dbus.h"

SecondDisplay::SecondDisplay(QObject *parent) : QObject(parent)
{
    display = new org::asteroid::display(SERVICE_NAME, DISPLAY_OBJECT, QDBusConnection::sessionBus(), this);
}

SecondDisplay::~SecondDisplay()
{
}

bool SecondDisplay::hasSecondDisplay() const
{
    if (!display->isValid()) {
        qDebug() << "No remote connection! Daemon not active?";
        return false;
    }

    return display->HasSecondDisplay();
}
bool SecondDisplay::synchronizeTime() const
{
    if (!display->isValid()) {
        qDebug() << "No remote connection! Daemon not active?";
        return false;
    }

    return display->SynchronizeTime();
}