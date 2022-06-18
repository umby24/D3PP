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
RUN cd lua-5.4.4
RUN wget https://gist.githubusercontent.com/dcarrith/899047f3a2d603b25a58/raw/7fa41ee5c0113ed721abe979c269afe48472baf5/Makefile.patch -O Makefile.patch --no-check-certificate
RUN patch -l < Makefile.patch && unexpand Makefile > Makefile.unexpanded && mv Makefile.unexpanded Makefile
RUN cd src/ && wget https://gist.githubusercontent.com/dcarrith/6095183b8dc60c909779/raw/614d769d5b70b69b5d67cfb618696bfb906e2aca/src.Makefile.patch -O Makefile.patch --no-check-certificate && patch -l < Makefile.patch && unexpand Makefile > Makefile.unexpanded && mv Makefile.unexpanded Makefile
RUN cd ../ && make all test && make install
RUN ln -s /usr/local/lib/liblua.so /lib/x86_64-linux-gnu/liblua-5.4.so


COPY ./run_cmake.sh /
COPY ./run_compile.sh /
