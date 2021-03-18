//
// Created by Wande on 3/7/2021.
//

#include "../include/Network.h"

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