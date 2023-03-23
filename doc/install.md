# Setting up DMD Reader on a pinball machine

## Get the hardware

TODO

## Connect the hardware

TODO

## Install Raspberry Pi base software

- Get the latest Raspberry Pi OS from https://www.raspberrypi.com/software/
- You don't need most of the software, therefore it's best to use "Raspberry Pi OS Lite - 64bit"
- Create a user, enable SSH and WiFi. You can do this in the "Advanced options" of the Raspberry Pi Imager.
- Write the software to the SD card
- Insert the SD card into the Raspberry Pi and boot
- Check that you can login via SSH

## Get DMD Reader

```
sudo apt update
sudo apt install -y git
git clone --recurse-submodules https://github.com/pinballpower/code_dmdreader
```

## Install the software

```
cd code_dmdreader
. ./install-pi-software.sh
. ./compile-pi.sh
```
