#pragma once

#include <stdint.h>
#include <cstring>

namespace Vi {
    //Non-owning raw buffer class
    struct Buffer {
        uint8_t* Data{ nullptr };
        uint64_t Size{ 0 };

        Buffer() = default;

        Buffer(uint64_t size) {
            allocate(size);
        }

        Buffer(const Buffer&) = default;

        static Buffer copy(Buffer other) {
            Buffer result(other.Size);
            memcpy_s(result.Data, result.Size, other.Data, other.Size);
            return result;
        }

        void allocate(uint64_t size) {
            release();

            Data = new uint8_t[size];
            Size = size;
        }

        void release() {
            delete[] Data;
            Data = nullptr;
            Size = 0;
        }

        template<typename T>
        T* as() {
            return reinterpret_cast<T*>(Data);
        }

        operator bool() const {
            return reinterpret_cast<bool>(Data);
        }
    };

    struct ScopedBuffer {
        ScopedBuffer(Buffer buffer): m_Buffer(buffer) {
        }

        ScopedBuffer(uint64_t size): m_Buffer(size) {
        }

        ~ScopedBuffer() {
            m_Buffer.release();
        }

        uint8_t* data() {
            return m_Buffer.Data;
        }

        uint64_t size() {
            return m_Buffer.Size;
        }

        template<typename T>
        T* as() {
            return m_Buffer.as<T>();
        }

        operator bool() const {
            return m_Buffer;
        }

    private:
        Buffer m_Buffer;
    };
}
