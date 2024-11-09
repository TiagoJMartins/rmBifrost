#!/bin/bash -Eeu

echo "::group::setup"
set -x
source /opt/codex/ferrari/4.0.813/environment-setup-cortexa53-crypto-remarkable-linux
mkdir build && cd build
set +x
echo "::endgroup::"

echo "::group::cmake"
cmake ../ -DCMAKE_BUILD_TYPE=Debug
cmake --build ./ -j8
echo "::endgroup::"
