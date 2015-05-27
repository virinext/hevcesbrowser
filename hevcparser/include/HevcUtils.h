#ifndef UTILS_H_
#define UTILS_H_

#include <cstddef>
#include <cstdint>
#include <memory>


namespace HEVC
{
  class Slice;
  class SPS;

  uint32_t log2(uint32_t k);
  uint32_t calcNumPocTotalCurr(std::shared_ptr<HEVC::Slice> pslice, std::shared_ptr<HEVC::SPS> psps);
}


#endif
