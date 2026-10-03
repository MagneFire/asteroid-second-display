// SPDX-FileCopyrightText: 2023-2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusVariant>
#include <QTextStream>
#include <functional>
#include <optional>

#include "types.h"

using namespace SecondDisplay;

namespace {

QTextStream err(stderr);
QTextStream out(stdout);

class Daemon
{
public:
    bool call(const char *path, const char *interface, const QString &method, const QVariantList &arguments,
              QVariant *result = nullptr) const
    {
        QDBusMessage message = QDBusMessage::createMethodCall(ServiceName, path, interface, method);
        message.setArguments(arguments);
        const QDBusMessage reply = QDBusConnection::sessionBus().call(message);
        if (reply.type() == QDBusMessage::ErrorMessage) {
            err << method << ": " << reply.errorMessage() << Qt::endl;
            return false;
        }
        if (result)
            *result = reply.arguments().value(0);
        return true;
    }

    bool callBool(const char *path, const char *interface, const QString &method,
                  const QVariantList &arguments = {}) const
    {
        QVariant ok;
        return call(path, interface, method, arguments, &ok) && ok.toBool();
    }

    std::optional<QVariant> property(const QString &name) const
    {
        QVariant value;
        if (!call(DisplayPath, "org.freedesktop.DBus.Properties", "Get", {DisplayInterface, name}, &value))
            return std::nullopt;
        return value.value<QDBusVariant>().variant();
    }

    bool setProperty(const QString &name, const QVariant &value) const
    {
        return call(DisplayPath, "org.freedesktop.DBus.Properties", "Set",
                    {DisplayInterface, name, QVariant::fromValue(QDBusVariant(value))})
            && property(name) == value;
    }
};

std::optional<Hand> parseHand(const QString &name)
{
    if (name == "minute")
        return Hand::Minute;
    if (name == "hour")
        return Hand::Hour;
    return std::nullopt;
}

std::optional<Rotation> parseRotation(const QString &name)
{
    if (name == "cw")
        return Rotation::Clockwise;
    if (name == "ccw")
        return Rotation::CounterClockwise;
    return std::nullopt;
}

struct Action
{
    QCommandLineOption option;
    unsigned int capability;
    std::function<bool(const Daemon &, const QString &)> run;
};

std::optional<bool> parseSwitch(const QString &value)
{
    if (value == "on")
        return true;
    if (value == "off")
        return false;
    return std::nullopt;
}

std::function<bool(const Daemon &, const QString &)> setToggle(const QString &property)
{
    return [property](const Daemon &daemon, const QString &argument) {
        const auto enabled = parseSwitch(argument);
        if (!enabled) {
            err << "Expected on or off, got " << argument << Qt::endl;
            return false;
        }
        return daemon.setProperty(property, *enabled);
    };
}

bool setBackground(const Daemon &daemon, const QString &argument)
{
    if (argument != "black" && argument != "white") {
        err << "Expected black or white, got " << argument << Qt::endl;
        return false;
    }
    const Background background = argument == "white" ? Background::White : Background::Black;
    return daemon.setProperty("DisplayColor", static_cast<int>(background));
}

bool moveHand(const Daemon &daemon, const QString &argument)
{
    const QStringList fields = argument.split(':');
    const auto hand = parseHand(fields.value(0));
    bool validPosition = false;
    const int position = fields.value(1).toInt(&validPosition);
    if (fields.size() != 2 || !hand || !validPosition) {
        err << "Expected <minute|hour>:<position>, got " << argument << Qt::endl;
        return false;
    }
    return daemon.callBool(HandsPath, HandsInterface, "MoveHand", {static_cast<int>(*hand), position});
}

bool calibrate(const Daemon &daemon, const QString &argument)
{
    const QStringList fields = argument.split(':');
    const auto hand = parseHand(fields.value(0));
    const auto rotation = parseRotation(fields.value(1));
    bool validSteps = false;
    const int steps = fields.value(2).toInt(&validSteps);
    if (fields.size() != 3 || !hand || !rotation || !validSteps) {
        err << "Expected <minute|hour>:<cw|ccw>:<steps>, got " << argument << Qt::endl;
        return false;
    }
    return daemon.callBool(HandsPath, HandsInterface, "Calibrate",
                           {static_cast<int>(*hand), static_cast<int>(*rotation), steps});
}

bool printPositions(const Daemon &daemon, const QString &)
{
    QVariant positions;
    if (!daemon.call(HandsPath, HandsInterface, "GetPositions", {}, &positions))
        return false;
    const QList<int> values = qdbus_cast<QList<int>>(positions);
    if (values.size() != 2)
        return false;
    out << "minute " << values[0] << ", hour " << values[1] << Qt::endl;
    return true;
}

}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription("Controls the second display through asteroid-seconddisplayd.");
    parser.addHelpOption();

    const std::vector<Action> actions{
        {{"sync-time", "Sync the second display with the system time."},
         Capability::TimeSync,
         [](const Daemon &daemon, const QString &) {
             return daemon.callBool(DisplayPath, DisplayInterface, "SynchronizeTime");
         }},
        {{"enter-timepiece", "Enter timepiece mode and power off."},
         Capability::TimepieceMode,
         [](const Daemon &daemon, const QString &) {
             return daemon.callBool(DisplayPath, DisplayInterface, "EnterTimepieceMode", {true});
         }},
        {{"prepare-timepiece", "Prepare timepiece mode without powering off."},
         Capability::TimepieceMode,
         [](const Daemon &daemon, const QString &) {
             return daemon.callBool(DisplayPath, DisplayInterface, "EnterTimepieceMode", {false});
         }},
        {{"step-counter", "Turn the step counter on or off.", "on|off"}, Capability::StepCounter,
         setToggle("StepCounterEnabled")},
        {{"heart-rate", "Turn the heart rate sensor on or off.", "on|off"}, Capability::HeartRate,
         setToggle("HeartRateEnabled")},
        {{"motion", "Turn motion on or off.", "on|off"}, Capability::Motion, setToggle("MotionEnabled")},
        {{"background", "Set the display background.", "black|white"}, Capability::DisplayColor, setBackground},
        {{"positions", "Print the hand positions."}, Capability::Hands, printPositions},
        {{"move-hand", "Move a hand, e.g. minute:90.", "hand:position"}, Capability::Hands, moveHand},
        {{"calibrate", "Move a hand and mark that spot as 12 o'clock, e.g. hour:cw:3.", "hand:rotation:steps"},
         Capability::Hands,
         calibrate},
        {{"resume-watch-mode", "Let the hands show the time again."},
         Capability::Hands,
         [](const Daemon &daemon, const QString &) {
             return daemon.callBool(HandsPath, HandsInterface, "ResumeWatchMode");
         }},
    };
    for (const Action &action : actions)
        parser.addOption(action.option);
    parser.process(app);

    std::vector<const Action *> requested;
    for (const Action &action : actions) {
        if (parser.isSet(action.option))
            requested.push_back(&action);
    }
    if (requested.empty())
        parser.showHelp(1);

    const Daemon daemon;
    const auto capabilities = daemon.property("Capabilities");
    if (!capabilities) {
        err << "asteroid-seconddisplayd is not running" << Qt::endl;
        return 1;
    }

    bool ok = true;
    for (const Action *action : requested) {
        const QString name = action->option.names().constFirst();
        if (!(capabilities->toUInt() & action->capability)) {
            err << "--" << name << " is not supported on this watch" << Qt::endl;
            ok = false;
        } else if (!action->run(daemon, action->option.valueName().isEmpty() ? QString() : parser.value(action->option))) {
            err << "--" << name << " failed" << Qt::endl;
            ok = false;
        }
    }
    return ok ? 0 : 1;
}
