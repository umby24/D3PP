FROM alpine

LABEL description="D3PP Classicube Server"

# install build dependencies
RUN apk add --no-cache lua5.4 sqlite-libs zlib

WORKDIR /opt/D3PP

COPY bin/Debug/D3PP D3PP
COPY bin/Debug/files.json files.json

EXPOSE 25565

CMD ["./D3PP"]