//
// Created by unknown on 8/19/21.
//

#ifndef D3PP_BYTEBUFFER_H
#define D3PP_BYTEBUFFER_H
#include <functional>
#include <mutex>
#include <string>
#include <atomic>

#include "TaskScheduler.h"

class ByteBuffer : public TaskItem {
public:
    ByteBuffer(std::function<void()> callback);
    ~ByteBuffer();
    void MainFunc();
    int Size() const;
    int ReadSize() const;
    // -- Read methods.
    unsigned char PeekByte();
    int PeekIntLE();
    unsigned char ReadByte();
    short ReadShort();
    int ReadInt();
    std::string ReadString();
    std::vector<unsigned char> ReadByteArray();
    std::vector<unsigned char> ReadByte(int length);
    // -- Write methods
    void Write(unsigned char value);
    void Write(short value);
    void Write(int value);
    void Write(std::string value);
    void Write(std::vector<unsigned char> memory, int length);
    // -- Control
    void Shift(int size);
    void Purge();
    std::vector<unsigned char> GetAllBytes();
protected:
    static const int initial_size = 1024; // -- Initial allocation size
    static const int block_size = 128; // -- Size it will be increased at each bump..
    std::vector<unsigned char> _buffer;
    std::atomic<unsigned int> _size;
    std::atomic<unsigned int> _readPos;
    std::atomic<unsigned int> _writePos;
    std::atomic<int> _largestAlloc = 0;
    std::mutex _bufLock;
private:

    void Resize(int size);
    std::function<void()> cbfunc;

};
#endif //D3PP_BYTEBUFFER_H
