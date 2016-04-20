#include "ConvToString.h"

#include <string>


std::string ConvToString::NALUnitType(HEVC::NALUnitType type)
{
  switch(type)
  {
    case HEVC::NAL_TRAIL_N:
      return "NAL_TRAIL_N";
      break;
    case HEVC::NAL_TRAIL_R:
      return "NAL_TRAIL_R";
      break;
    case HEVC::NAL_TSA_N:
      return "NAL_TSA_N";
      break;
    case HEVC::NAL_TSA_R:
      return "NAL_TSA_R";
      break;
    case HEVC::NAL_STSA_N:
      return "NAL_STSA_N";
      break;
    case HEVC::NAL_STSA_R:
      return "NAL_STSA_R";
      break;
    case HEVC::NAL_RADL_N:
      return "NAL_RADL_N";
      break;
    case HEVC::NAL_RADL_R:
      return "NAL_RADL_R";
      break;
    case HEVC::NAL_RASL_N:
      return "NAL_RASL_N";
      break;
    case HEVC::NAL_RASL_R:
      return "NAL_RASL_R";
      break;
    case HEVC::NAL_BLA_W_LP:
      return "NAL_BLA_W_LP";
      break;
    case HEVC::NAL_BLA_W_RADL:
      return "NAL_BLA_W_RADL";
      break;
    case HEVC::NAL_BLA_N_LP:
      return "NAL_BLA_N_LP";
      break;
    case HEVC::NAL_IDR_W_RADL:
      return "NAL_IDR_W_RADL";
      break;
    case HEVC::NAL_IDR_N_LP:
      return "NAL_IDR_N_LP";
      break;
    case HEVC::NAL_CRA_NUT:
      return "NAL_CRA_NUT";
      break;
    case HEVC::NAL_IRAP_VCL23:
      return "NAL_IRAP_VCL23";
      break;
    case HEVC::NAL_VPS:
      return "NAL_VPS";
      break;
    case HEVC::NAL_SPS:
      return "NAL_SPS";
      break;
    case HEVC::NAL_PPS:
      return "NAL_PPS";
      break;
    case HEVC::NAL_AUD:
      return "NAL_AUD";
      break;
    case HEVC::NAL_EOS_NUT:
      return "NAL_EOS_NUT";
      break;
    case HEVC::NAL_EOB_NUT:
      return "NAL_EOB_NUT";
      break;
    case HEVC::NAL_FD_NUT:
      return "NAL_FD_NUT";
      break;
    case HEVC::NAL_SEI_PREFIX:
      return "NAL_SEI_PREFIX";
      break;
    case HEVC::NAL_SEI_SUFFIX:
      return "NAL_SEI_SUFFIX";
      break;
    default:
      return "INVALID";
      break;
  }
}
