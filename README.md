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

On hoki the daemon talks to the Sidekick graphics HAL over `/dev/hwbinder`. Timepiece mode uploads a face to the BG co-processor, switches it to the time-only firmware and hands it the panel before the watch powers off. With AOD offload enabled the BG also draws the always-on display.

The BG only takes the always-on display over once a client has described the watchface with `SetFace`; until then, and after `ClearFace`, the compositor keeps drawing the always-on display itself. Timepiece mode uses the described face too and falls back to the generic face from `assets/hoki`. A description names a background PNG, a strip PNG with the glyphs 0 to 9 stacked vertically, the top-left positions of the hour and minute digits, optionally a second strip for the minutes, a colon PNG with its position and whether the face is in colour:

```json
{
  "background": "/run/user/1000/face/background.png",
  "digits": "/run/user/1000/face/digits.png",
  "minuteDigits": "/run/user/1000/face/minute-digits.png",
  "hoursX": 40, "hoursY": 148,
  "minutesX": 220, "minutesY": 148,
  "colon": "/run/user/1000/face/colon.png",
  "colonX": 198, "colonY": 148,
  "color": false
}
```

The BG has a 16 entry palette with 1-bit alpha: 16 grey levels for a grey face, 16 colours otherwise.
