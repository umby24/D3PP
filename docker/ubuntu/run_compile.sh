#!/bin/sh

set -e

cd /tmp/D3PP
make -j $(nproc)