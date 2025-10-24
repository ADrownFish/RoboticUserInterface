#include "robotic_user_interface/core/ByteUtils.h"

namespace robot
{

/**
 * @brief 获取字节的某一个bit状态
 * @param byte 需要读取的地址
 * @param pos 需要读取的位置，注意 0 是起始位置
 * @return true or false
 */
bool getBit(void *byte, uint8_t pos)
{
  uint8_t *ptr = (uint8_t *)byte;
  uint8_t byteValue = ptr[pos / 8];
  uint8_t bitPos = pos % 8;
  return (byteValue & (1 << bitPos)) != 0;
}
/**
 * @brief 写入字节的某一个bit状态
 * @param byte 需要写入的地址
 * @param pos 需要写入的位置，注意 0 是起始位置
 * @param value 值
 * @return None
 */
void setBit(void *byte, uint8_t pos, bool value)
{
  uint8_t *ptr = (uint8_t *)byte;
  uint8_t byteValue = ptr[pos / 8];
  uint8_t bitPos = pos % 8;

  if (value)
  {
    byteValue |= (1 << bitPos);
  }
  else
  {
    byteValue &= ~(1 << bitPos);
  }

  ptr[pos / 8] = byteValue;
}
/**
 * @brief 设置字节的某一个bit状态为true
 * @param byte 需要写入的地址
 * @param pos 需要写入的位置，注意 0 是起始位置
 * @return None
 */
void setBitEnable(void *byte, uint8_t pos)
{
  uint8_t *ptr = (uint8_t *)byte;
  ptr[pos / 8] |= (1 << (pos % 8));
}
/**
 * @brief 设置字节的某一个bit状态为false
 * @param byte 需要写入的地址
 * @param pos 需要写入的位置，注意 0 是起始位置
 * @return None
 */
void setBitDisable(void *byte, uint8_t pos)
{
  uint8_t *ptr = (uint8_t *)byte;
  ptr[pos / 8] &= ~(1 << (pos % 8));
}

void printByteArray(const uint8_t *array, size_t size)
{
  std::cout << std::hex << std::setw(2) << std::setfill('0');
  for (size_t i = 0; i < size; ++i)
  {
    std::cout  << static_cast<int>(array[i]) << " ";
  }
  std::cout << std::dec << std::endl;
}

std::string formatByteArray(const uint8_t *array, size_t size)
{
  std::stringstream ss;
  ss << std::hex << std::setw(2) << std::setfill('0');
  for (size_t i = 0; i < size; ++i) {
      ss << std::setw(2) << static_cast<int>(array[i]) << " ";
  }
  ss << std::dec;
  return ss.str();
}


int8_t toInt8(const uint8_t *p) {
  BytesTrans_1 converter;
  converter.uint8_ = p[0];
  return converter.int8_;
}

int16_t toInt16(const uint8_t *p, bool le) {
  BytesTrans_2 converter;
  if (le) {
    converter.uint8_[0] = p[0];
    converter.uint8_[1] = p[1];
  } else {
    converter.uint8_[0] = p[1];
    converter.uint8_[1] = p[0];
  }
  return converter.int16_;
}

int32_t toInt32(const uint8_t *p, bool le) {
  BytesTrans_4 converter;
  if (le) {
    converter.uint8_[0] = p[0];
    converter.uint8_[1] = p[1];
    converter.uint8_[2] = p[2];
    converter.uint8_[3] = p[3];
  } else {
    converter.uint8_[0] = p[3];
    converter.uint8_[1] = p[2];
    converter.uint8_[2] = p[1];
    converter.uint8_[3] = p[0];
  }
  return converter.int32_;
}

int64_t toInt64(const uint8_t *p, bool le){
  BytesTrans_8 converter;
  if (le) {
    converter.uint8_[0] = p[0];
    converter.uint8_[1] = p[1];
    converter.uint8_[2] = p[2];
    converter.uint8_[3] = p[3];
    converter.uint8_[4] = p[4];
    converter.uint8_[5] = p[5];
    converter.uint8_[6] = p[6];
    converter.uint8_[7] = p[7];
  } else {
    converter.uint8_[0] = p[7];
    converter.uint8_[1] = p[6];
    converter.uint8_[2] = p[5];
    converter.uint8_[3] = p[4];
    converter.uint8_[4] = p[3];
    converter.uint8_[5] = p[2];
    converter.uint8_[6] = p[1];
    converter.uint8_[7] = p[0];
  }
  return converter.int64_;
}

uint8_t toUint8(const uint8_t *p) {
  BytesTrans_1 converter;
  converter.uint8_ = p[0];
  return converter.uint8_;
}

uint16_t toUint16(const uint8_t *p, bool le) {
  BytesTrans_2 converter;
  if (le) {
    converter.uint8_[0] = p[0];
    converter.uint8_[1] = p[1];
  } else {
    converter.uint8_[0] = p[1];
    converter.uint8_[1] = p[0];
  }
  return converter.uint16_;
}

uint32_t toUint32(const uint8_t *p, bool le) {
  BytesTrans_4 converter;
  if (le) {
    converter.uint8_[0] = p[0];
    converter.uint8_[1] = p[1];
    converter.uint8_[2] = p[2];
    converter.uint8_[3] = p[3];
  } else {
    converter.uint8_[0] = p[3];
    converter.uint8_[1] = p[2];
    converter.uint8_[2] = p[1];
    converter.uint8_[3] = p[0];
  }
  return converter.uint32_;
}

uint64_t toUint64(const uint8_t *p, bool le){
  BytesTrans_8 converter;
  if (le) {
    converter.uint8_[0] = p[0];
    converter.uint8_[1] = p[1];
    converter.uint8_[2] = p[2];
    converter.uint8_[3] = p[3];
    converter.uint8_[4] = p[4];
    converter.uint8_[5] = p[5];
    converter.uint8_[6] = p[6];
    converter.uint8_[7] = p[7];
  } else {
    converter.uint8_[0] = p[7];
    converter.uint8_[1] = p[6];
    converter.uint8_[2] = p[5];
    converter.uint8_[3] = p[4];
    converter.uint8_[4] = p[3];
    converter.uint8_[5] = p[2];
    converter.uint8_[6] = p[1];
    converter.uint8_[7] = p[0];
  }
  return converter.uint64_;
}

float toFloat32(const uint8_t *p, bool le) {
  BytesTrans_4 converter;
  if (le) {
    converter.uint8_[0] = p[0];
    converter.uint8_[1] = p[1];
    converter.uint8_[2] = p[2];
    converter.uint8_[3] = p[3];
  } else {
    converter.uint8_[0] = p[3];
    converter.uint8_[1] = p[2];
    converter.uint8_[2] = p[1];
    converter.uint8_[3] = p[0];
  }
  return converter.float32_;
}

double toFloat64(const uint8_t *p, bool le) {
  BytesTrans_8 converter;
  if (le) {
    converter.uint8_[0] = p[0];
    converter.uint8_[1] = p[1];
    converter.uint8_[2] = p[2];
    converter.uint8_[3] = p[3];
    converter.uint8_[4] = p[4];
    converter.uint8_[5] = p[5];
    converter.uint8_[6] = p[6];
    converter.uint8_[7] = p[7];
  } else {
    converter.uint8_[0] = p[7];
    converter.uint8_[1] = p[6];
    converter.uint8_[2] = p[5];
    converter.uint8_[3] = p[4];
    converter.uint8_[4] = p[3];
    converter.uint8_[5] = p[2];
    converter.uint8_[6] = p[1];
    converter.uint8_[7] = p[0];
  }
  return converter.float64_;
}

  } // namespace sevnce