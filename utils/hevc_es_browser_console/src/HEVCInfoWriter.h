#ifndef HEVC_INFO_WRITER_H_
#define HEVC_INFO_WRITER_H_

#include <HevcParser.h>

#include <vector>
#include <map>
#include <ostream>

class HEVCInfoWriter: public HEVC::Parser::Consumer
{
  public:
    virtual void onNALUnit(std::shared_ptr<HEVC::NALUnit> pNALUnit, const HEVC::Parser::Info *pInfo);

    void write(std::ostream &out);

  protected:

    void writeVPS(std::shared_ptr<HEVC::VPS> pVPS, std::ostream &out);
    void writeSPS(std::shared_ptr<HEVC::SPS> pSPS, std::ostream &out);
    void writePPS(std::shared_ptr<HEVC::PPS> pPPS, std::ostream &out);
    void writeSlice(std::shared_ptr<HEVC::Slice> pSlice, std::ostream &out);

    struct NALUInfo
    {
      std::shared_ptr<HEVC::NALUnit>     m_pNALUnit;
      HEVC::Parser::Info                 m_info;
    };
    
    std::vector<NALUInfo>    m_nalus;


    std::map<uint32_t, std::shared_ptr<HEVC::VPS> >          m_vpsMap;
    std::map<uint32_t, std::shared_ptr<HEVC::SPS> >          m_spsMap;
    std::map<uint32_t, std::shared_ptr<HEVC::PPS> >          m_ppsMap;

};


#endif
