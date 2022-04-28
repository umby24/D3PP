FROM ubuntu:impish

LABEL description="D3PP Classicube Server"

# install build dependencies
RUN apt-get update && apt-get -y install sqlite3 liblua5.4-0 libc6 libstdc++6
RUN ln -s /lib/x86_64-linux-gnu/liblua5.4.so.0.0.0 /lib/x86_64-linux-gnu/liblua-5.4.so

WORKDIR /opt/D3PP

COPY bin/Debug/D3PP D3PP
COPY bin/Debug/files.json files.json
COPY bin/Debug/Data/Command.txt Data/Command.txt
COPY bin/Debug/Data/Block.json Data/Block.json
COPY bin/Debug/Data/Build_Mode.txt Data/Build_Mode.txt
COPY bin/Debug/Data/Map_List.txt Data/Map_List.txt
COPY bin/Debug/Lua/ Lua/
COPY bin/Debug/Maps/ Maps/

EXPOSE 25565

CMD ["./D3PP"]