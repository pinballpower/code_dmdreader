# Setting up DMD Reader on a pinball machine

## Get the hardware

TODO

## Connect the hardware

TODO

## Install Raspberry Pi base software

- Get the latest Raspberry Pi OS from https://www.raspberrypi.com/software/
- Create a user, enable SSH and WiFi. You can do this in the "Advanced options" of the Raspberry Pi Imager.
- Write the software to the SD card
- Insert the SD card into the Raspberry Pi and boot
- Check that you can login via SSH

## Get DMD Reader

```
sudo apt updata
sudo apt install -f git
git clone https://github.com/pinballpower/code_dmdreader
```
