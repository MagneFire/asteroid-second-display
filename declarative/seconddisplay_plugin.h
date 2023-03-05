#ifndef SECOND_DISPLAY_PLUGIN_H
#define SECOND_DISPLAY_PLUGIN_H

#include <QQmlExtensionPlugin>

class SecondDisplayPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.asteroid.seconddisplay")

public:
    explicit SecondDisplayPlugin(QObject *parent = 0);

    virtual void registerTypes(const char *uri);
};

#endif // SECOND_DISPLAY_PLUGIN_H

