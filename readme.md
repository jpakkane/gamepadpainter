# Gamepad Painter experiment

A simple painting application that takes pen tilt and pressure information
from a gamepad rather than a drawing tablet. This allows you to control
pressure and tilt with a different hand than drawing. The app does not require
a drawing tablet, you can use a mouse instead.

- Left stick controls tilt
- Left trigger controls pressure
- Left shoulder button toggles tilt lock
- Gamepad button Y (the top one) clears screen

Tested to work on Linux. Should compile on Windows and possibly even macOS
out of the box as the SDL dependency is downloaded and build automatically
by using Meson's [WrapDB](https://wrapdb.mesonbuild.com). Not tested on those
platforms, though.

