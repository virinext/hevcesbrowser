#include "BitstreamReader.h"

#include <climits>
#include <stdexcept>
#include <cassert>
#include <boost/config/no_tr1/complex.hpp>

#include "HevcUtils.h"

BitstreamReader::BitstreamReader(const uint8_t *ptr, std::size_t size):
  m_ptr(ptr)
  ,m_size(size)
  ,m_posBase(0)
  ,m_posInBase(CHAR_BIT - 1)
{
}

std::size_t BitstreamReader::available()
{
  return (m_size - m_posBase -1) * CHAR_BIT + m_posInBase + 1;
}

bool BitstreamReader::getBit()
{
  if(m_posBase >= m_size)
    throw std::runtime_error("BitstreamReader: not enought data");

  bool res = m_ptr[m_posBase] & 1 << m_posInBase;

  m_posInBase--;

  if(m_posInBase > CHAR_BIT)
  {
    m_posInBase = CHAR_BIT-1;
    m_posBase++;

    if(m_posBase >= 2)
    {
      if(m_ptr[m_posBase - 2] == 0 && m_ptr[m_posBase - 1] == 0 && m_ptr[m_posBase] == 3)
        m_posBase++;
    }
  }

  return res;
}


uint32_t BitstreamReader::getBits(std::size_t num)
{
  assert(num <= 32);

  uint32_t result = 0;
  for(std::size_t i=0; i<num; i++)
    if(getBit())
      result |= 1 << (num - i - 1);

  return result;
}


uint32_t BitstreamReader::showBits(std::size_t num)
{
  assert(num <= 32);

  std::size_t posBasePrev = m_posBase;
  std::size_t posInBasePrev = m_posInBase;

  uint32_t result = 0;
  for(std::size_t i=0; i<num; i++)
    if(getBit())
      result |= 1 << (num - i - 1);

  m_posBase = posBasePrev;
  m_posInBase = posInBasePrev;

  return result;
}



uint32_t BitstreamReader::getGolombU()
{
/*  uint32_t buf = showBits(32);
  uint32_t log = 31 - HEVC::log2(buf);
  getBits(log);

    uint32_t dRet;
*/

  long numZeroBits = -1;
  for(long bit = 0; !bit; numZeroBits++)
    bit = getBit();

  return (1 << numZeroBits) - 1 + getBits(numZeroBits);
}



int32_t BitstreamReader::getGolombS()
{
  int32_t buf = getGolombU();

  if (buf & 1)
    buf = (buf + 1) >> 1;
  else
    buf = -(buf >> 1);

  return buf;
}

