sudo apt update -y
sudo apt install -y iotop ninja-build git libssl-dev
sudo apt install -y --no-install-recommends raspberrypi-ui-mods lxterminal gvfs
sudo apt install -y libx11-dev libxcursor-dev libxinerama-dev libxrandr-dev libxi-dev libasound2-dev mesa-common-dev libgl1-mesa-dev libgbm-dev libavutil-dev libavformat-dev libavfilter-dev
sudo apt install -y libstb-dev libglbinding-dev libglfw3-dev libglm-dev libfmt-dev libglew-dev libdrm-dev libboost-all-dev g++ libraspberrypi-dev raspberrypi-kernel-headers
sudo apt install -y autoconf autoconf-archive libtool libkmod-dev pkg-config
LIBGPIODVERSION=1.6.3
wget https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/snapshot/libgpiod-$LIBGPIODVERSION.tar.gz
tar xvfz libgpiod-$LIBGPIODVERSION.tar.gz
rm libgpiod-$LIBGPIODVERSION.tar.gz
cd libgpiod-$LIBGPIODVERSION
./autogen.sh --enable-tools=yes
make
make install
cd ..
rm -rf libgpiod-$LIBGPIODVERSION
