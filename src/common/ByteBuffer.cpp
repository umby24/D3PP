//
// Created by unknown on 8/19/21.
//
#include "common/ByteBuffer.h"
#include <cmath>
#include "Utils.h"


ByteBuffer::ByteBuffer(const std::function<void()>& callback) : _buffer(initial_size), _bufLock() {
    this->_size = (unsigned int) ByteBuffer::initial_size;

    if (callback != nullptr)
        this->cbfunc = callback;

    this->Interval = std::chrono::seconds(10);
    this->LastRun = std::chrono::system_clock::now();
    this->Main = [this] { MainFunc(); };
    _readPos = 0;
    _writePos = 0;
    _largestAlloc = 0;
   // std::string myId = TaskScheduler::RegisterTask("ByteBuffer", *this);
   // this->TaskId = myId;
}

void ByteBuffer::MainFunc() {
    if ((_largestAlloc > ByteBuffer::initial_size) && (_writePos > ByteBuffer::initial_size || _readPos > ByteBuffer::initial_size)) {
        _largestAlloc = 0;
        return;
    }
    // -- We can resize to save memory space.
    if (_writePos == 0 && _readPos == 0) {
        bool didLock = _bufLock.try_lock();
        if (didLock) {
            if (_buffer.size() == 0 || _buffer.data() == nullptr) {
                return; // -- In the process of being dealloced
            }
            _buffer.resize(initial_size);
            _size = initial_size;
            _bufLock.unlock();
        }
    }
    _largestAlloc = 0;
}

int ByteBuffer::Size() const {
    return (int)this->_writePos;
}

unsigned char ByteBuffer::PeekByte() {
    unsigned char result = 0;
    {
        const std::scoped_lock<std::mutex> pqlock(_bufLock);
        result = _buffer.at(_readPos);
    }
    return result;
}

int ByteBuffer::PeekIntLE() {
    int result = 0;
    const std::scoped_lock<std::mutex> pqlock(_bufLock);
    result |= _buffer.at(_readPos);
    result |= _buffer.at(_readPos + 1l) << 8;
    result |= _buffer.at(_readPos + 2) << 16;
    result |= _buffer.at(_readPos + 3) << 24;
    return result;
}

unsigned char ByteBuffer::ReadByte() {
    unsigned char result = 0;
    const std::scoped_lock<std::mutex> pqlock(_bufLock);
    result = _buffer.at(_readPos++);
    return result;
}

short ByteBuffer::ReadShort() {
    short result = 0;
    const std::scoped_lock<std::mutex> pqlock(_bufLock);
    result |= _buffer.at(_readPos++) << 8;
    result |= _buffer.at(_readPos++);
    return result;
}

int ByteBuffer::ReadInt() {
    int result = 0;
    const std::scoped_lock<std::mutex> pqlock(_bufLock);
    result |= _buffer.at(_readPos++) << 24;
    result |= _buffer.at(_readPos++) << 16;
    result |= _buffer.at(_readPos++) << 8;
    result |= _buffer.at(_readPos++);
    return result;
}

std::string ByteBuffer::ReadString() {
    const std::scoped_lock<std::mutex> pqlock(_bufLock);
    std::string result(_buffer.begin()+_readPos, _buffer.begin()+_readPos+64);
    _readPos += 64;
    Utils::TrimString(result);
    return result;
}

std::vector<unsigned char> ByteBuffer::ReadByteArray() {
    const std::scoped_lock<std::mutex> pqlock(_bufLock);
    std::vector<unsigned char> result(_buffer.begin()+_readPos, _buffer.begin()+_readPos+1024);
    _readPos += 1024;
    return result;
}

std::vector<unsigned char> ByteBuffer::ReadByte(int length) {
    const std::scoped_lock<std::mutex> pqlock(_bufLock);
    std::vector<unsigned char> result(_buffer.begin()+_readPos, _buffer.begin()+_readPos+length);
    _readPos += length;
    return result;
}

void ByteBuffer::Write(unsigned char value) {
    const std::scoped_lock<std::mutex> pqlock(_bufLock);
    Resize(1);
    _buffer.at(_writePos++) = value;
}

void ByteBuffer::Write(short value) {
    const std::scoped_lock<std::mutex> pqlock(_bufLock);
    Resize(2);
    _buffer.at(_writePos++) = (unsigned char) (value >> 8);
    _buffer.at(_writePos++) = (unsigned char)value;
}

void ByteBuffer::Write(int value) {
    const std::scoped_lock<std::mutex> pqlock(_bufLock);
    Resize(4);
    _buffer.at(_writePos++) = (unsigned char) (value >> 24);
    _buffer.at(_writePos++) = (unsigned char) (value >> 16);
    _buffer.at(_writePos++) = (unsigned char) (value >> 8);
    _buffer.at(_writePos++) = (unsigned char)value;
}

void ByteBuffer::Write(std::string value) {
    const std::scoped_lock<std::mutex> pqlock(_bufLock);
    Resize(64);
    if (value.size() < 64) {
        std::string padded(64-value.size(), ' ');
        value += padded;
    }
    const auto * constStr = reinterpret_cast<const unsigned char *> (value.c_str());
    _buffer.insert(_buffer.begin()+_writePos, value.begin(), value.end());
    _writePos += 64;
}

void ByteBuffer::Write(std::vector<unsigned char> memory, int length) {
    int actualLen = length;
    if (memory.size() != length) {
        actualLen = memory.size();
    }
    const std::scoped_lock<std::mutex> pqlock(_bufLock);
    Resize(actualLen);
    _buffer.insert(_buffer.begin()+_writePos, memory.begin(), memory.begin()+actualLen);
    _writePos += actualLen;
}

void ByteBuffer::Shift(int size) {
    const std::scoped_lock<std::mutex> pqlock(_bufLock);
    std::copy(_buffer.begin()+size, _buffer.end(), _buffer.begin());
    _readPos -= size;
    _writePos -= size;
}

void ByteBuffer::Purge() {
    if (cbfunc) {
        cbfunc();
    }
}

std::vector<unsigned char> ByteBuffer::GetAllBytes() {
    const std::scoped_lock<std::mutex> pqlock(_bufLock);
    std::vector<unsigned char> result(_buffer.begin(), _buffer.begin()+_writePos);
    _writePos = 0;
    _readPos = 0; // -- Not in the OG but I think it's needed..
    return result;
}

void ByteBuffer::Resize(int size) {
    if (size> _largestAlloc)
        _largestAlloc = size;

    if ((this->_size - this->_writePos) < size) {
        int blockMultiplier = std::ceil(size/(float)block_size);
        int newSize = (block_size * blockMultiplier) + _size;
        _buffer.resize(newSize);
        _largestAlloc = newSize - _size;
        _size = newSize;
    }
}

ByteBuffer::~ByteBuffer() {
    TaskScheduler::UnregisterTask(this->TaskId);
    _writePos = 0;
    _readPos = 0;
    _size = 0;
    _buffer.clear();
}

int ByteBuffer::ReadSize() const {
    return _writePos - _readPos;
}
