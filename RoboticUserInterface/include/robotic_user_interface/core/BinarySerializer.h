#pragma once

/**
 * BinarySerializer
 *  序列化和反序列化类
 *
 * @author ADrownFish
 * @author a.drownfish@qq.com
 * @date 2025-10-24
 */

#include <cstdint>
#include <cstddef>
#include <cstring>

#include "ByteUtils.h"

namespace robot
{

class BinarySerializer {
private:
  uint8_t* m_ptr;
  size_t m_offset;

public:
  BinarySerializer(uint8_t* buffer) 
      : m_ptr(buffer), m_offset(0){}
  
  // 获取当前偏移量
  size_t offset() const { return m_offset; }
  
  // 设置偏移量
  void seek(size_t newOffset) { m_offset = newOffset; }

  uint8_t* data() const { return m_ptr; }

  uint8_t* current() const { return m_ptr + m_offset; }

  BinarySerializer& writeInt8(int8_t value) {
    m_ptr[m_offset] = static_cast<uint8_t>(value);
    m_offset += sizeof(int8_t);
    return *this;
  }
  
  BinarySerializer& writeUint8(uint8_t value) {
    m_ptr[m_offset] = value;
    m_offset += sizeof(uint8_t);
    return *this;
  }

  BinarySerializer& writeInt16(int16_t value, bool le = true) {
    if (le) {
        m_ptr[m_offset] = static_cast<uint8_t>(value & 0xFF);
        m_ptr[m_offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    } else {
        m_ptr[m_offset] = static_cast<uint8_t>((value >> 8) & 0xFF);
        m_ptr[m_offset + 1] = static_cast<uint8_t>(value & 0xFF);
    }
    m_offset += sizeof(int16_t);
    return *this;
  }
  
  BinarySerializer& writeUint16(uint16_t value, bool le = true) {
    if (le) {
        m_ptr[m_offset] = static_cast<uint8_t>(value & 0xFF);
        m_ptr[m_offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    } else {
        m_ptr[m_offset] = static_cast<uint8_t>((value >> 8) & 0xFF);
        m_ptr[m_offset + 1] = static_cast<uint8_t>(value & 0xFF);
    }
    m_offset += sizeof(uint16_t);
    return *this;
  }

  BinarySerializer& writeInt32(int32_t value, bool le = true) {
    if (le) {
        m_ptr[m_offset] = static_cast<uint8_t>(value & 0xFF);
        m_ptr[m_offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
        m_ptr[m_offset + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
        m_ptr[m_offset + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
    } else {
        m_ptr[m_offset] = static_cast<uint8_t>((value >> 24) & 0xFF);
        m_ptr[m_offset + 1] = static_cast<uint8_t>((value >> 16) & 0xFF);
        m_ptr[m_offset + 2] = static_cast<uint8_t>((value >> 8) & 0xFF);
        m_ptr[m_offset + 3] = static_cast<uint8_t>(value & 0xFF);
    }
    m_offset += sizeof(int32_t);
    return *this;
  }
  
  BinarySerializer& writeUint32(uint32_t value, bool le = true) {
    if (le) {
        m_ptr[m_offset] = static_cast<uint8_t>(value & 0xFF);
        m_ptr[m_offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
        m_ptr[m_offset + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
        m_ptr[m_offset + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
    } else {
        m_ptr[m_offset] = static_cast<uint8_t>((value >> 24) & 0xFF);
        m_ptr[m_offset + 1] = static_cast<uint8_t>((value >> 16) & 0xFF);
        m_ptr[m_offset + 2] = static_cast<uint8_t>((value >> 8) & 0xFF);
        m_ptr[m_offset + 3] = static_cast<uint8_t>(value & 0xFF);
    }
    m_offset += sizeof(uint32_t);
    return *this;
  }

  BinarySerializer& writeFloat32(float value, bool le = true) {
    uint32_t intValue;
    static_assert(sizeof(float) == sizeof(uint32_t), "float must be 32 bits");
    std::memcpy(&intValue, &value, sizeof(float));
    
    if (le) {
        for (size_t i = 0; i < sizeof(uint32_t); ++i) {
        m_ptr[m_offset + i] = static_cast<uint8_t>((intValue >> (i * 8)) & 0xFF);
        }
    } else {
        for (size_t i = 0; i < sizeof(uint32_t); ++i) {
        m_ptr[m_offset + i] = static_cast<uint8_t>((intValue >> ((sizeof(uint32_t) - 1 - i) * 8)) & 0xFF);
        }
    }
    m_offset += sizeof(float);
    return *this;
  }
  
  BinarySerializer& writeFloat64(double value, bool le = true) {
    uint64_t intValue;
    static_assert(sizeof(double) == sizeof(uint64_t), "double must be 64 bits");
    std::memcpy(&intValue, &value, sizeof(double));
    
    if (le) {
        for (size_t i = 0; i < sizeof(uint64_t); ++i) {
        m_ptr[m_offset + i] = static_cast<uint8_t>((intValue >> (i * 8)) & 0xFF);
        }
    } else {
        for (size_t i = 0; i < sizeof(uint64_t); ++i) {
        m_ptr[m_offset + i] = static_cast<uint8_t>((intValue >> ((sizeof(uint64_t) - 1 - i) * 8)) & 0xFF);
        }
    }
    m_offset += sizeof(double);
    return *this;
  }

  BinarySerializer& writeInt64(int64_t value, bool le = true) {
    if (le) {
        for (size_t i = 0; i < sizeof(int64_t); ++i) {
        m_ptr[m_offset + i] = static_cast<uint8_t>((value >> (i * 8)) & 0xFF);
        }
    } else {
        for (size_t i = 0; i < sizeof(int64_t); ++i) {
        m_ptr[m_offset + i] = static_cast<uint8_t>((value >> ((sizeof(int64_t) - 1 - i) * 8)) & 0xFF);
        }
    }
    m_offset += sizeof(int64_t);
    return *this;
  }
  
  BinarySerializer& writeUint64(uint64_t value, bool le = true) {
    if (le) {
        for (size_t i = 0; i < sizeof(uint64_t); ++i) {
        m_ptr[m_offset + i] = static_cast<uint8_t>((value >> (i * 8)) & 0xFF);
        }
    } else {
        for (size_t i = 0; i < sizeof(uint64_t); ++i) {
        m_ptr[m_offset + i] = static_cast<uint8_t>((value >> ((sizeof(uint64_t) - 1 - i) * 8)) & 0xFF);
        }
    }
    m_offset += sizeof(uint64_t);
    return *this;
  }
  
  // 原始数据写入
  BinarySerializer& writeBytes(const uint8_t* data, size_t length) {
  std::memcpy(m_ptr + m_offset, data, length);
  m_offset += length;
  return *this;
  }
  
  BinarySerializer& writeString(const char* str) {
    size_t len = std::strlen(str);
    return writeBytes(reinterpret_cast<const uint8_t*>(str), len);
  }

  int8_t readInt8() {
    return toInt8(m_ptr + m_offset++); 
  }

  uint8_t readUint8() {
    return toUint8(m_ptr + m_offset++); 
  }

  int16_t readInt16(bool le = true) {
    auto v = toInt16(m_ptr + m_offset, le);
    m_offset += sizeof(int16_t);
    return v;
  }

  uint16_t readUint16(bool le = true) {
    auto v = toUint16(m_ptr + m_offset, le);
    m_offset += sizeof(uint16_t);
    return v;
  }

  int32_t readInt32(bool le = true) {
    auto v = toInt32(m_ptr + m_offset, le);
    m_offset += sizeof(int32_t);
    return v;
  }

  uint32_t readUint32(bool le = true) {
    auto v = toUint32(m_ptr + m_offset, le);
    m_offset += sizeof(uint32_t);
    return v;
  }

  float readFloat32(bool le = true) {
    auto v = toFloat32(m_ptr + m_offset, le);
    m_offset += sizeof(float);
    return v;
  }

  double readFloat64(bool le = true) {
    auto v = toFloat64(m_ptr + m_offset, le);
    m_offset += sizeof(double);
    return v;
  }

  int64_t readInt64(bool le = true) {
    auto v = toInt64(m_ptr + m_offset, le);
    m_offset += sizeof(int64_t);
    return v;
  }

  uint64_t readUint64(bool le = true) {
    auto v = toUint64(m_ptr + m_offset, le);
    m_offset += sizeof(uint64_t);
    return v;
  }
  
  // 跳过指定字节
  BinarySerializer& skip(size_t bytes) {
    m_offset += bytes;
    return *this;
  }
  
  // 重置序列化器
  void reset(uint8_t* newBuffer = nullptr) {
    if (newBuffer != nullptr) {
        m_ptr = newBuffer;
    }
    m_offset = 0;
  }
};

}