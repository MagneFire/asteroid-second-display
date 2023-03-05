# AsteroidOS Second Display

This project adds support for devices that have a second display (additional LCD, physical hands).

## Development

A DBus interface is used to achieve communication between the `asteroid-seconddisplayd` daemon and any (QML) application.

To ease up on the development header files are provided that can be used to create the DBus specification files (xml) that ends up being the API between the daemon and applications.
Generate/update the `xml` files by executing: `./dbus/generate_xml.sh`