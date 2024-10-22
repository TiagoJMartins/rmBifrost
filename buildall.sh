#!/bin/bash -Eeux

set -o pipefail

# git submodule update --init --recursive

RMPP_HOST=rmpp-remote

docker run \
  --rm -it \
  -v "$PWD:/src" -w /src \
  --platform linux/amd64 \
  --entrypoint /src/build-entrypoint.sh \
  eeems/remarkable-toolchain:latest-rmpp

scp build/lib/libbifrost.so "$RMPP_HOST":~/

xochitl_pid=$(ssh "$RMPP_HOST" "pgrep xochitl" || true)
if [ -n "$xochitl_pid" ]; then
  ssh "$RMPP_HOST" "kill $xochitl_pid"
fi

ssh "$RMPP_HOST" "LD_PRELOAD=./libbifrost.so /usr/bin/xochitl --system"
