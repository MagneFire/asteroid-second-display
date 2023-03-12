#include "seconddisplay.h"

#include "display_interface.h"

#include "dbus.h"

SecondDisplay::SecondDisplay(QObject *parent) : QObject(parent)
{
    display = new org::asteroid::display(SERVICE_NAME, DISPLAY_OBJECT, QDBusConnection::systemBus(), this);

    QObject::connect(display, &org::asteroid::display::StepCounterChanged, this, &SecondDisplay::stepCounterChanged);
}

SecondDisplay::~SecondDisplay()
{
}

bool SecondDisplay::hasConnection() const
{
    if (!display->isValid()) {
        qDebug() << "No remote connection! Daemon not active?";
        return false;
    }
    return true;
}

bool SecondDisplay::hasSecondDisplay() const
{
    if (!hasConnection()) return false;

    return display->HasSecondDisplay();
}
bool SecondDisplay::synchronizeTime() const
{
    if (!hasConnection()) return false;

    return display->SynchronizeTime();
}

bool SecondDisplay::hasStepCounter() const
{
    if (!hasConnection()) return false;

    return display->HasStepCounter();
}

bool SecondDisplay::getStepCounter() const
{
    if (!hasConnection()) return false;

    return display->GetStepCounter();
}
bool SecondDisplay::setStepCounter(bool enabled) const
{
    if (!hasConnection()) return false;

    return display->SetStepCounter(enabled);
}