// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QtEndian>

#include "sidekickclient.h"
#include "timepieceface.h"

using namespace SecondDisplay;
using namespace SecondDisplay::Sidekick;

namespace {

QTextStream out(stdout);
QTextStream err(stderr);

constexpr char Usage[] = R"(Commands, run in the given order:
  list                     List the services on /dev/hwbinder.
  caps                     Print the HAL capabilities, display size and free bytes.
  bytes                    Print the bytes available for resources.
  color-format [0|1|2]     Print or set the color format (RGB_332, RGB_565, GRAY).
  framebuffer <file>       Save the BG framebuffer to a file.
  reset                    Drop all resources and state. Needs --really.
  upload <png> [id]        Upload a PNG as a drawable at 0,0 (default id 1).
  delete <id>...           Delete resources.
  face [12|24]             Upload the timepiece face from the asset directory.
  display-begin [full|idle|off]
                           Let the BG draw the uploaded resources.
                           The panel must be off or in doze.
  display-end              Hand the panel back to the AP.
  time                     Call updateDisplayTime.
  brightness <bright> <dim>
                           Set fixed brightness values.
  twm-config <ms> <0|1>    Set the TWM bright time and tilt-to-bright.
  twm-prepare              Load the time-only firmware. Needs --allow-firmware-switch.
                           A reboot restores the normal firmware.
  twm-enter                Enter time-only mode. Needs --really and a blank panel.
                           Power off afterwards.)";

bool readPng(const QString &path, QByteArray *png, DrawableInfo *info)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        err << "Unable to read " << path << Qt::endl;
        return false;
    }
    *png = file.readAll();
    if (png->size() < 24 || !png->startsWith("\x89PNG")) {
        err << path << " is not a PNG" << Qt::endl;
        return false;
    }
    info->width = qFromBigEndian<quint32>(png->constData() + 16);
    info->height = qFromBigEndian<quint32>(png->constData() + 20);
    return true;
}

std::optional<DisplayPowerState> parsePowerState(const QString &name)
{
    if (name.isEmpty() || name == "off")
        return DisplayPowerState::Off;
    if (name == "idle")
        return DisplayPowerState::Idle;
    if (name == "full")
        return DisplayPowerState::Full;
    return std::nullopt;
}

bool report(const char *name, const SidekickClient::Result &result)
{
    out << name << ": " << result.toString() << Qt::endl;
    return result.ok();
}

}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QString("Talks to the Sidekick graphics HAL directly, without asteroid-seconddisplayd.\n\n%1").arg(Usage));
    parser.addHelpOption();
    parser.addPositionalArgument("command", "Command and its arguments.", "<command> [args...]");
    const QCommandLineOption assetsOption("assets", "Directory with the face PNGs.", "dir", HOKI_FACE_DIR);
    const QCommandLineOption serviceOption("service", "HIDL service name.", "name", DefaultService);
    const QCommandLineOption firmwareOption("allow-firmware-switch", "Allow twm-prepare.");
    const QCommandLineOption reallyOption("really", "Allow twm-enter.");
    parser.addOptions({assetsOption, serviceOption, firmwareOption, reallyOption});
    parser.process(app);

    QStringList args = parser.positionalArguments();
    if (args.isEmpty())
        parser.showHelp(1);
    const QString command = args.takeFirst();

    if (command == "list") {
        for (const QString &name : SidekickClient::listServices())
            out << name << Qt::endl;
        return 0;
    }

    SidekickClient client(parser.value(serviceOption));
    if (!client.connectToService()) {
        err << "Service " << client.serviceName() << " is not available" << Qt::endl;
        return 1;
    }

    if (command == "caps") {
        const auto caps = client.getCapabilities();
        if (!caps)
            return 1;
        out << "capabilities 0x" << Qt::hex << caps->capabilities << Qt::dec << Qt::endl
            << "display " << caps->displayWidth << "x" << caps->displayHeight << Qt::endl
            << "bytes available " << caps->bytesAvailable << Qt::endl
            << "color bits " << caps->color.redBits << "/" << caps->color.greenBits << "/" << caps->color.blueBits
            << ", palette " << caps->color.paletteSize << ", alpha " << caps->color.oneBitAlpha << "/"
            << caps->color.multiBitAlpha << Qt::endl;
        return 0;
    }
    if (command == "bytes") {
        const auto bytes = client.getBytesAvailable();
        if (!bytes)
            return 1;
        out << *bytes << Qt::endl;
        return 0;
    }
    if (command == "color-format") {
        if (!args.isEmpty())
            return report("setColorFormat", client.setColorFormat(static_cast<ColorFormat>(args[0].toUInt()))) ? 0 : 1;
        const auto format = client.getColorFormat();
        if (!format)
            return 1;
        out << static_cast<int>(*format) << Qt::endl;
        return 0;
    }
    if (command == "framebuffer") {
        if (args.isEmpty())
            parser.showHelp(1);
        const auto data = client.readFramebuffer();
        if (!data)
            return 1;
        QFile file(args[0]);
        if (!file.open(QIODevice::WriteOnly) || file.write(*data) != data->size()) {
            err << "Unable to write " << args[0] << Qt::endl;
            return 1;
        }
        out << data->size() << " bytes" << Qt::endl;
        return 0;
    }
    if (command == "reset") {
        if (!parser.isSet(reallyOption)) {
            err << "reset drops every resource, pass --really" << Qt::endl;
            return 1;
        }
        return report("reset", client.reset()) ? 0 : 1;
    }
    if (command == "upload") {
        if (args.isEmpty())
            parser.showHelp(1);
        QByteArray png;
        DrawableInfo info;
        if (!readPng(args.value(0), &png, &info))
            return 1;
        info.id = args.value(1, "1").toInt();
        info.type = DrawableType::Generic;
        info.displayInTwm = true;
        if (!report("beginResources", client.beginResources()))
            return 1;
        const bool ok = report("sendBitmapPng8888", client.sendBitmapPng8888(info, png));
        report("endResources", client.endResources());
        return ok ? 0 : 1;
    }
    if (command == "delete") {
        QList<std::uint32_t> ids;
        for (const QString &arg : args)
            ids.append(arg.toUInt());
        if (ids.isEmpty())
            parser.showHelp(1);
        if (!report("beginResources", client.beginResources()))
            return 1;
        const bool ok = report("deleteResources", client.deleteResources(ids));
        report("endResources", client.endResources());
        return ok ? 0 : 1;
    }
    if (command == "face") {
        const auto caps = client.getCapabilities();
        if (!caps)
            return 1;
        const auto face = loadTimepieceFace(parser.value(assetsOption), args.value(0) == "12", caps->displayWidth);
        if (!face)
            return 1;
        if (!report("beginResources", client.beginResources()))
            return 1;
        bool ok = report("sendFontPng8888", client.sendFontPng8888(face->font, face->fontPng));
        ok = ok && report("sendBitmapPng8888 background", client.sendBitmapPng8888(face->background.drawable, face->background.png));
        ok = ok && report("sendBitmapPng8888 colon", client.sendBitmapPng8888(face->colon.drawable, face->colon.png));
        ok = ok && report("sendNumberResource hours", client.sendNumberResource(face->hours.drawable, face->hours.number));
        ok = ok && report("sendNumberResource minutes", client.sendNumberResource(face->minutes.drawable, face->minutes.number));
        ok = report("endResources", client.endResources()) && ok;
        if (!ok)
            report("deleteResources", client.deleteResources(face->ids()));
        return ok ? 0 : 1;
    }
    if (command == "display-begin") {
        const auto state = parsePowerState(args.value(0));
        if (!state)
            parser.showHelp(1);
        return report("beginDisplay", client.beginDisplay(*state)) ? 0 : 1;
    }
    if (command == "display-end")
        return report("endDisplay", client.endDisplay(DisplayPowerState::Full)) ? 0 : 1;
    if (command == "time")
        return report("updateDisplayTime", client.updateDisplayTime()) ? 0 : 1;
    if (command == "brightness") {
        if (args.size() != 2)
            parser.showHelp(1);
        const auto bright = static_cast<std::int16_t>(args[0].toInt());
        const auto dim = static_cast<std::int16_t>(args[1].toInt());
        if (!report("setBrightness", client.setBrightness(true, {}, {}, {bright}, {dim})))
            return 1;
        return report("setAlsMode", client.setAlsMode(AlsMode::Off, 0, 0)) ? 0 : 1;
    }
    if (command == "twm-config") {
        if (args.size() != 2)
            parser.showHelp(1);
        return report("setTwmConfig", client.setTwmConfig(args[0].toUInt(), args[1].toInt() != 0)) ? 0 : 1;
    }
    if (command == "twm-prepare") {
        if (!parser.isSet(firmwareOption)) {
            err << "twm-prepare switches the BG firmware, pass --allow-firmware-switch" << Qt::endl;
            return 1;
        }
        return report("prepareTWM", client.prepareTwm()) ? 0 : 1;
    }
    if (command == "twm-enter") {
        if (!parser.isSet(reallyOption)) {
            err << "twm-enter hands the panel to the BG for good, pass --really" << Qt::endl;
            return 1;
        }
        return report("enterTwm", client.enterTwm()) ? 0 : 1;
    }
    err << "Unknown command " << command << Qt::endl;
    return 1;
}
