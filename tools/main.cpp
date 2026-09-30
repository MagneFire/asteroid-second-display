// SPDX-FileCopyrightText: 2023 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include <QCommandLineParser>
#include <QCoreApplication>

#include "display_interface.h"
#include "hands_interface.h"

#include "types.h"

using namespace SecondDisplay;

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

    auto display = new org::asteroid::SecondDisplay::Display(ServiceName, DisplayPath, QDBusConnection::sessionBus());
    auto hands = new org::asteroid::SecondDisplay::Hands(ServiceName, HandsPath, QDBusConnection::sessionBus());

    if (!display->isValid()) {
        qDebug() << "No remote connection! Daemon not active?";
        return false;
    }
    if (!hands->isValid()) {
        qDebug() << "No remote connection! Daemon not active?";
        return false;
    }

    if (!display->capabilities()) {
        qCritical() << "This device does not support second display functionalities.";
    }

    if (parser.isSet(syncTime)) {
        display->SynchronizeTime().waitForFinished();
    }
    if (parser.isSet(enterTimepiece)) {
        display->EnterTimepieceMode(false).waitForFinished();
    }

    if (parser.isSet(enableStepCounter)) {
        display->setStepCounterEnabled(true);
    }
    if (parser.isSet(disableStepCounter)) {
        display->setStepCounterEnabled(false);
    }
}