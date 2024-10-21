#!/bin/bash -Eeux

source /opt/codex/ferrari/4.0.813/environment-setup-cortexa53-crypto-remarkable-linux
mkdir -p build
cd build || exit
cmake ../ -DCMAKE_BUILD_TYPE=Debug
cmake --build ./ -j8
