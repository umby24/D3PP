//
// Created by unknown on 4/2/21.
//

#ifndef D3PP_COMPRESSION_H
#define D3PP_COMPRESSION_H
#include <string>
#include <zlib.h>
#include <fstream>

#include "Logger.h"

class GZIP {
public:
    static int GZip_CompressBound(int inputLen);

    static int GZip_Compress(unsigned char *output, int outputLen, unsigned char *input, int inputLen);

    static int GZip_Decompress(unsigned char *output, int outputLen, unsigned char *input, int inputLen);

    static bool GZip_CompressToFile(unsigned char *input, int inputLen, std::string filename);

    static int GZip_DecompressFromFile(unsigned char *output, int outputLen, std::string filename);
};
#endif //D3PP_COMPRESSION_H
