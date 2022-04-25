sudo apt -y update
sudo apt -y install iotop ninja-build git libssl-dev
vcpkg/bootstrap-vcpkg.sh
wget https://github.com/Kitware/CMake/releases/download/v3.23.0/cmake-3.23.0.tar.gz
tar xvfz cmake-3.23.0.tar.gz
cd cmake-3.23.0
./configure
make -j 4
sudo make install
cd ..
rm -rf cmake-3.23.0
sudo apt-get install -y --no-install-recommends raspberrypi-ui-mods lxterminal gvfs
sudo apt-get install -y libx11-dev libxcursor-dev libxinerama-dev libxrandr-dev libxi-dev libasound2-dev mesa-common-dev libgl1-mesa-dev 
sudo apt-get install -y libstb-dev libglbinding-dev libglfw3-dev libglm-dev libfmt.dev libglew-dev libdrm-dev libboost-all-dev

