#pragma once

#include <stdint.h>
#include <cstdint> 
#include <cstring> // for memcpy
#include <string>
#include <iostream>
#include <iomanip> 
#include <sstream> 

namespace robot
{
  /**
   * @brief 获取字节的某一个bit状态
   * @param byte 需要读取的地址
   * @param pos 需要读取的位置，注意 0 是起始位置
   * @return true or false
   */
  bool getBit(void *byte, uint8_t pos);
  /**
   * @brief 写入字节的某一个bit状态
   * @param byte 需要写入的地址
   * @param pos 需要写入的位置，注意 0 是起始位置
   * @param value 值
   * @return None
   */
  void setBit(void *byte, uint8_t pos, bool value);
  /**
   * @brief 设置字节的某一个bit状态为true
   * @param byte 需要写入的地址
   * @param pos 需要写入的位置，注意 0 是起始位置
   * @return None
   */
  void setBitEnable(void *byte, uint8_t pos);
  /**
   * @brief 设置字节的某一个bit状态为false
   * @param byte 需要写入的地址
   * @param pos 需要写入的位置，注意 0 是起始位置
   * @return None
   */
  void setBitDisable(void *byte, uint8_t pos);
  /**
   * @brief 打印字节数组
   * @param array 地址
   * @param size 大小
   * @return None
   */
  void printByteArray(const uint8_t *array, size_t size);
  /**
   * @brief 字节数组 转 字符串
   * @param array 地址
   * @param size 大小
   * @return 字符串
   */
  std::string formatByteArray(const uint8_t *array, size_t size);

  /**
   * @brief 字节数组 转 值
   * @param p 地址
   * @param le littleEndian 是否小端模式,反之大端
   * @return 值
   */
  // =================================== 转换 ===================================
  
  int8_t   toInt8(const uint8_t *p);
  int16_t  toInt16(const uint8_t *p, bool le = true);
  int32_t  toInt32(const uint8_t *p, bool le = true);
  int64_t  toInt64(const uint8_t *p, bool le = true);
  uint8_t  toUint8(const uint8_t *p);
  uint16_t toUint16(const uint8_t *p, bool le = true);
  uint32_t toUint32(const uint8_t *p, bool le = true);
  uint64_t toUint64(const uint8_t *p, bool le = true);
  float    toFloat32(const uint8_t *p, bool le = true);
  double   toFloat64(const uint8_t *p, bool le = true);

  union BytesTrans_2
  {
    int8_t   int8_[2];
    uint8_t  uint8_[2];

    uint16_t uint16_;
    int16_t  int16_;
  };
  union BytesTrans_1
  {
    int8_t   int8_;
    uint8_t  uint8_;
  };
  union BytesTrans_4
  {
    int8_t   int8_[4];
    uint8_t  uint8_[4];

    uint16_t uint16_[2];
    int16_t  int16_[2];

    uint32_t uint32_;
    int32_t  int32_;

    float    float32_;
  };
  union BytesTrans_8
  {
    int8_t   int8_[8];
    uint8_t  uint8_[8];

    uint16_t uint16_[4];
    int16_t  int16_[4];

    uint32_t uint32_[2];
    int32_t  int32_[2];

    uint64_t uint64_;
    int64_t  int64_;

    float    float32_[2];
    double   float64_;
  };

}