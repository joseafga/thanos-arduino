# Thanos

Robot for UNIP Assis project!

It receives commands from [another bluetooth device](https://github.com/joseafga/thanos-python) and do some actions.  
In this project basically use commands to control a H-bridge and buzzer but can be anything.

## Require
- HC-06 module (or similar)
- Arduino IDE

## Install and Use
Open `hc06_control.ino` in your Arduino IDE and flash it.
In `parseData` function we have a switch-case to do the action according to the input. You will probably want to change that (or maybe not).

## License
**MIT License**  
See [LICENSE](LICENSE) file for details.
