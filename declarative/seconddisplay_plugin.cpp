#include "seconddisplay_plugin.h"
#include <QtQml>
#include <SecondDisplay>
#include <Hands>

SecondDisplayPlugin::SecondDisplayPlugin(QObject *parent) : QQmlExtensionPlugin(parent)
{
}

void SecondDisplayPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<SecondDisplay>(uri, 1,0, "SecondDisplay");
    qmlRegisterType<Hands>(uri, 1,0, "Hands");
}

