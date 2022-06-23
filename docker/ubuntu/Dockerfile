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
  zlib1g-dev \
  libreadline-dev

RUN wget http://ftp.us.debian.org/debian/pool/main/l/lua5.4/liblua5.4-0_5.4.2-2_amd64.deb
RUN wget http://ftp.us.debian.org/debian/pool/main/l/lua5.4/liblua5.4-dev_5.4.2-2_amd64.deb
RUN dpkg -i liblua5.4-0_5.4.2-2_amd64.deb && dpkg -i liblua5.4-dev_5.4.2-2_amd64.deb

RUN ln -s /usr/lib/x86_64-linux-gnu/liblua5.4.so.0.0.0 /lib/x86_64-linux-gnu/liblua-5.4.so


COPY ./run_cmake.sh /
COPY ./run_compile.sh /
