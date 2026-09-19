#!/usr/bin/env bash
set -e

echo "=== 1. Adding Repositories for GCC 16 & Latest CMake ==="
sudo apt-get update
sudo apt-get install -y software-properties-common wget gpg ca-certificates

# Ubuntu Toolchain PPA (GCC 16用)
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y

# Kitware Official Repository (最新 CMake用)
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null

echo "=== 2. Installing Toolchain, CMake & SFML Dependencies ==="
sudo apt-get update
sudo apt-get install -y --no-install-recommends \
    gcc-16 \
    g++-16 \
    cmake \
    ninja-build \
    gdb \
    lldb \
    libsfml-dev \
    libgl1-mesa-dev \
    libgl1-mesa-dri \
    x11-apps

# システムのデフォルトコンパイラを GCC 16 に設定
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-16 100 \
  --slave /usr/bin/g++ g++ /usr/bin/g++-16 \
  --slave /usr/bin/gcov gcov /usr/bin/gcov-16

echo "=== 3. Setting up WSLg X11 Socket Link ==="
sudo rm -rf /tmp/.X11-unix
if [ -d "/mnt/wslg/.X11-unix" ]; then
    sudo ln -s /mnt/wslg/.X11-unix /tmp/.X11-unix
fi

sudo rm -rf /tmp/.X11-unix
sudo ln -s /tmp/wslg-x11 /tmp/.X11-unix

echo "=== Setup Completed Successfully ==="