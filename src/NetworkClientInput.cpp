//
// Created by Wande on 3/7/2021.
//

#include "../include/Network.h"

int InputBufferAvailable(NetworkClient* client) {
    return client->InputBufferAvailable;
}

void InputAddOffset(NetworkClient* client, int bytes) {
    client->InputBufferOffset += bytes;
    client->InputBufferAvailable -= bytes;

    if (client->InputBufferOffset < 0)
        client->InputBufferOffset += NETWORK_BUFFER_SIZE;

    if (client->InputBufferOffset >= NETWORK_BUFFER_SIZE)
        client->InputBufferOffset -= NETWORK_BUFFER_SIZE;
}

char InputReadByte(NetworkClient* client) {
    char result;
    if (client->InputBufferAvailable >= 1) {
        result = client->InputBuffer[client->InputBufferOffset];
        client->InputBufferOffset += 1;
        client->InputBufferAvailable -= 1;
        if (client->InputBufferOffset >= NETWORK_BUFFER_SIZE)
            client->InputBufferOffset -= NETWORK_BUFFER_SIZE;
    }
    return result;
}

short InputReadShort(NetworkClient* client) {
    short result;
    if (client->InputBufferAvailable >= 2) {
        result |= client->InputBuffer[client->InputBufferOffset] << 8;
        result |= client->InputBuffer[client->InputBufferOffset];
        client->InputBufferAvailable -= 2;
        client->InputBufferOffset += 2;
        if (client->InputBufferOffset >= NETWORK_BUFFER_SIZE)
            client->InputBufferOffset -= NETWORK_BUFFER_SIZE;
    }
    return result;
}

int InputReadInt(NetworkClient* client) {
    int result;
    if (client->InputBufferAvailable >= 4) {
        result |= client->InputBuffer[client->InputBufferOffset] << 24; // -- Stealing this logic from something Im going to refactor this to later.
        result |= client->InputBuffer[client->InputBufferOffset] << 16; // -- OG D3 uses x86 ASM to achieve this.
        result |= client->InputBuffer[client->InputBufferOffset] << 8;
        result |= client->InputBuffer[client->InputBufferOffset];
        client->InputBufferAvailable -= 4;
        client->InputBufferOffset += 4;
        if (client->InputBufferOffset >= NETWORK_BUFFER_SIZE)
            client->InputBufferOffset -= NETWORK_BUFFER_SIZE;
    }
    return result;
}

std::string InputReadString(NetworkClient* client) {
    if (client->InputBufferAvailable >= 64) {
        std::string result(client->InputBuffer+client->InputBufferOffset, client->InputBuffer+client->InputBufferOffset+64);
        client->InputBufferAvailable -= 64;
        client->InputBufferOffset += 64;
        if (client->InputBufferOffset >= NETWORK_BUFFER_SIZE)
            client->InputBufferOffset -= NETWORK_BUFFER_SIZE;
        return result;
    }
    return std::string("");
}

void InputReadBytes(NetworkClient* client, char* buffer, int dataSize) {
    int dataRead = 0;
    while (dataRead < dataSize) {
        int maxData = NETWORK_BUFFER_SIZE - (client->InputBufferOffset);
        int dataTempSize = dataSize - dataRead;
        if (dataTempSize > maxData)
            dataTempSize = maxData;
        memcpy(client->InputBuffer+client->InputBufferOffset, buffer+dataRead, dataTempSize);
        dataRead += dataTempSize;
        client->InputBufferOffset += dataTempSize;
        client->InputBufferAvailable -= dataTempSize;
        if (client->InputBufferOffset >= NETWORK_BUFFER_SIZE)
            client->InputBufferOffset -= NETWORK_BUFFER_SIZE;
    }
}