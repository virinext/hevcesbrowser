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
    virtual void onWarning(const std::string &warning, const HEVC::Parser::Info *pInfo, HEVC::Parser::WarningType) {};

    void write(std::ostream &out);

  protected:

    struct NALUInfo
    {
      std::shared_ptr<HEVC::NALUnit>     m_pNALUnit;
      HEVC::Parser::Info                 m_info;
    };

    virtual void writeNALHeader(const NALUInfo& naluInfo, std::ostream &out);
    virtual void writeVPS(std::shared_ptr<HEVC::VPS> pVPS, std::ostream &out);
    virtual void writeSPS(std::shared_ptr<HEVC::SPS> pSPS, std::ostream &out);
    virtual void writePPS(std::shared_ptr<HEVC::PPS> pPPS, std::ostream &out);
    virtual void writeSlice(std::shared_ptr<HEVC::Slice> pSlice, std::ostream &out);
    virtual void writeAUD(std::shared_ptr<HEVC::AUD> pAUD, std::ostream &out);
    virtual void writeSEI(std::shared_ptr<HEVC::SEI> pSEI, std::ostream &out);

    virtual void writeProfileTierLevel(const HEVC::ProfileTierLevel &ptl, std::ostream &out, const std::string &prefix);
    virtual void writeHrdParameters(const HEVC::HrdParameters &hrd, uint8_t commonInfPresentFlag, std::ostream &out, const std::string &prefix);
    virtual void writeSubLayerHrdParameters(const HEVC::SubLayerHrdParameters &slhrd, uint8_t sub_pic_hrd_params_present_flag, std::ostream &out, const std::string &prefix);
    virtual void writeShortTermRefPicSet(std::size_t stRpsIdx, std::size_t num_short_term_ref_pic_sets, const std::vector<HEVC::ShortTermRefPicSet> &refPicSets, std::ostream &out, const std::string &prefix);
    virtual void writeVuiParameters(const HEVC::VuiParameters &vui, std::size_t sps_max_sub_layers_minus1, std::ostream &out, const std::string &prefix);

    std::vector<NALUInfo>    m_nalus;

    std::map<uint32_t, std::shared_ptr<HEVC::VPS> >          m_vpsMap;
    std::map<uint32_t, std::shared_ptr<HEVC::SPS> >          m_spsMap;
    std::map<uint32_t, std::shared_ptr<HEVC::PPS> >          m_ppsMap;

};


#endif
