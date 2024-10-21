#!/bin/bash -Eeux

set -o pipefail

# git submodule update --init --recursive

docker run \
  --rm -it \
  -v "$PWD:/src" -w /src \
  --platform linux/amd64 \
  --entrypoint /src/build-entrypoint.sh \
  eeems/remarkable-toolchain:latest-rmpp

scp build/lib/libbifrost.so rmpp:~/

xochitl_pid=$(ssh rmpp "pgrep xochitl")
if [ -n "$xochitl_pid" ]; then
  ssh rmpp "kill $xochitl_pid"
fi

ssh rmpp "LD_PRELOAD=./libbifrost.so /usr/bin/xochitl --system"
