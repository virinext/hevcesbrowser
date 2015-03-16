#ifndef CONV_TO_STRING
#define CONV_TO_STRING


#include "Hevc.h"


class ConvToString
{
  public:
    static std::string NALUnitType(HEVC::NALUnitType type);
};

#endif
