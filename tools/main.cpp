
#include <QObject>

#include "display_interface.h"
#include "hands_interface.h"

#include "dbus.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription("asteroid-second-display utility.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption syncTime("sync", "Synchronize time between the system and the second display.");
    QCommandLineOption enterTimepiece("enter-timepiece", "Enter timepiece mode, may shutdown the device.");
    QCommandLineOption enableStepCounter("enable-stepcounter", "Enable the stepcounter on the second display.");
    QCommandLineOption disableStepCounter("disable-stepcounter", "Disable the stepcounter on the second display.");

    parser.addOption(syncTime);
    parser.addOption(enterTimepiece);
    parser.addOption(enableStepCounter);
    parser.addOption(disableStepCounter);

    parser.process(app);

    if (argc <= 1) {
        parser.showHelp();
        return 0;
    }

    auto display = new org::asteroid::display(SERVICE_NAME, DISPLAY_OBJECT, QDBusConnection::systemBus());
    auto hands = new org::asteroid::hands(SERVICE_NAME, HANDS_OBJECT, QDBusConnection::systemBus());

    if (!display->isValid()) {
        qDebug() << "No remote connection! Daemon not active?";
        return false;
    }
    if (!hands->isValid()) {
        qDebug() << "No remote connection! Daemon not active?";
        return false;
    }

    if (!display->HasSecondDisplay()) {
        qCritical() << "This device does not support second display functionalities.";
    }

    if (parser.isSet(syncTime)) {
        display->SynchronizeTime().waitForFinished();
    }
    if (parser.isSet(enterTimepiece)) {
        display->EnterTimepieceMode().waitForFinished();
    }

    if (parser.isSet(enableStepCounter)) {
        display->SetStepCounter(true).waitForFinished();
    }
    if (parser.isSet(disableStepCounter)) {
        display->SetStepCounter(false).waitForFinished();
    }
}