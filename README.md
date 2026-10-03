# AsteroidOS Second Display

This project adds support for devices that have a second display (additional LCD, physical hands) or a co-processor that can show the time while the main CPU is off.

## Development

A DBus interface is used to achieve communication between the `asteroid-seconddisplayd` daemon and any (QML) application.

| Machine | Backend | Capabilities |
| --- | --- | --- |
| koi, medaka | `backend_casio` | time sync, timepiece mode, display color |
| catfish, rubyfish | `backend_mobvoi` (libhybris) | time sync, timepiece mode (catfish), step counter, heart rate, motion |
| narwhal | `backend_narwhal` | time sync, hands |
| hoki | `backend_hoki` (libgbinder) | time sync, timepiece mode |

On hoki the daemon talks to the Sidekick graphics HAL over `/dev/hwbinder`. Timepiece mode uploads the face from `assets/hoki` to the BG co-processor, switches it to the time-only firmware and hands it the panel before the watch powers off. `asteroid-seconddisplay-hoki-probe` runs the same calls one at a time for bring-up.
