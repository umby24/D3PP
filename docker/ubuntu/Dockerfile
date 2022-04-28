ARG version
FROM ubuntu:${version}

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

RUN apt-get update && apt-get install -y \
  build-essential \
  wget \
  cmake \
  git-core \
  sqlite3 \
  libsqlite3-dev \
  libc6 \
  libstdc++6 \
  zlib1g-dev

RUN wget https://www.lua.org/ftp/lua-5.4.4.tar.gz
RUN tar -zxf lua-5.4.4.tar.gz
RUN cd lua-5.4.4 && make all test && make install
RUN ln -s /lib/x86_64-linux-gnu/liblua5.4.so.0.0.0 /lib/x86_64-linux-gnu/liblua-5.4.so

COPY ./run_cmake.sh /
COPY ./run_compile.sh /
