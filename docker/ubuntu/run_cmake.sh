#!/bin/sh

set -e

mkdir -p /tmp/D3PP
cd /tmp/D3PP
cmake -DLUA_INCLUDE_DIR=/usr/include/lua5.4 -DLUA_LIBRARY=/usr/lib/x86_64-linux-gnu/liblua5.4.so --target D3PP /D3PP