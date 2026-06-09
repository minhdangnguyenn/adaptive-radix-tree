#pragma once

#include <assert.h>
#include <cstring>
#include <memory>
#include <stdint.h>

class Key {
  public:
    static constexpr uint32_t MAX_KEY_LENGTH = 256;
    static constexpr uint32_t STACK_LEN = 128;
    uint32_t len = 0;

    uint8_t *data;

    uint8_t stackKey[STACK_LEN];

    Key(uint64_t k) { setInt(k); }

    void setInt(uint64_t k) {
        data = stackKey;
        len = 8;
        *reinterpret_cast<uint64_t *>(stackKey) = __builtin_bswap64(k);
    }

    Key() {}

    ~Key();

    Key(const Key &key) = delete;

    Key(Key &&key);

    void set(const char bytes[], const std::size_t length);

    void operator=(const char key[]);

    bool operator==(const Key &k) const {
        if (k.getKeyLen() != getKeyLen()) {
            return false;
        }
        return std::memcmp(&k[0], data, getKeyLen()) == 0;
    }

    uint8_t &operator[](std::size_t i);

    const uint8_t &operator[](std::size_t i) const;

    uint32_t getKeyLen() const;

    void setKeyLen(uint32_t len);
};

inline uint8_t &Key::operator[](std::size_t i) {
    // assert(i < len);
    return data[i];
}

inline const uint8_t &Key::operator[](std::size_t i) const {
    // assert(i < len);
    return data[i];
}

inline uint32_t Key::getKeyLen() const { return len; }

inline Key::~Key() {
    if (len > STACK_LEN) {
        delete[] data;
        data = nullptr;
    }
}

inline Key::Key(Key &&key) {
    len = key.len;
    if (len > STACK_LEN) {
        data = key.data;
        key.data = nullptr;
    } else {
        memcpy(stackKey, key.stackKey, key.len);
        data = stackKey;
    }
}

inline void Key::set(const char bytes[], const std::size_t length) {
    if (len > STACK_LEN) {
        delete[] data;
    }
    if (length <= STACK_LEN) {
        memcpy(stackKey, bytes, length);
        data = stackKey;
    } else {
        data = new uint8_t[length];
        memcpy(data, bytes, length);
    }
    len = length;
}

inline void Key::operator=(const char key[]) {
    if (len > STACK_LEN) {
        delete[] data;
    }
    len = strlen(key);
    if (len <= STACK_LEN) {
        memcpy(stackKey, key, len);
        data = stackKey;
    } else {
        data = new uint8_t[len];
        memcpy(data, key, len);
    }
}

inline void Key::setKeyLen(uint32_t newLen) {
    if (len == newLen)
        return;
    if (len > STACK_LEN) {
        delete[] data;
    }
    len = newLen;
    if (len > STACK_LEN) {
        data = new uint8_t[len];
    } else {
        data = stackKey;
    }
}
