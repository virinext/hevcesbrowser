#include "Hevc.h"

#include <string.h>

using namespace HEVC;

NALUnit::NALUnit(NALUnitType type):
  m_nalUnitType(type)
{
}


NALUnit::~NALUnit()
{
}


NALUnitType NALUnit::getType() const
{
  return m_nalUnitType;
}
 
 
std::shared_ptr<NALUnit> NALUnit::copy() const
{
  std::shared_ptr<NALUnit> res;

  switch(m_nalUnitType)
  {
    case NAL_VPS:
    {
      res = std::shared_ptr<NALUnit>(new VPS(*((VPS *) this)));
      break;
    }

    case NAL_SPS:
    {
      res = std::shared_ptr<NALUnit>(new SPS(*((SPS *) this)));
      break;
    }
    
    case NAL_PPS:
    {
      res = std::shared_ptr<NALUnit>(new PPS(*((PPS *) this)));
      break;
    }

    case NAL_TRAIL_R:
    case NAL_TRAIL_N:
    case NAL_TSA_N:
    case NAL_TSA_R:
    case NAL_STSA_N:
    case NAL_STSA_R:
    case NAL_BLA_W_LP:
    case NAL_BLA_W_RADL:
    case NAL_BLA_N_LP:
    case NAL_IDR_W_RADL:
    case NAL_IDR_N_LP:
    case NAL_CRA_NUT:
    case NAL_RADL_N:
    case NAL_RADL_R:
    case NAL_RASL_N:
    case NAL_RASL_R:
    {
      res = std::shared_ptr<NALUnit>(new Slice(*((Slice *) this)));
      break;
    }

    default:
      res = std::shared_ptr<NALUnit>(new NALUnit(*this));
  };

  return res;
}


bool HEVC::VPS::operator == (const HEVC::VPS &vps) const
{
  if(this == &vps)
    return true;

  return memcmp(this, &vps, sizeof(HEVC::VPS)) == 0;
}


bool HEVC::SPS::operator == (const HEVC::SPS &sps) const
{
  if(this == &sps)
    return true;

  return memcmp(this, &sps, sizeof(HEVC::SPS)) == 0;
}


bool HEVC::PPS::operator == (const HEVC::PPS &pps) const
{
  if(this == &pps)
    return true;

  return memcmp(this, &pps, sizeof(HEVC::PPS)) == 0;
}

