#ifndef PROFILE_CONFORMANCE_ANALYZER_H_
#define PROFILE_CONFORMANCE_ANALYZER_H_

#include <QString>

#include <HevcParser.h>

#include <memory>
#include <map>


class ProfileConformanceAnalyzer:
  public HEVC::Parser::Consumer
{
  public:
    ProfileConformanceAnalyzer();

    virtual void onNALUnit(std::shared_ptr<HEVC::NALUnit> pNALUnit, const HEVC::Parser::Info *pInfo);
    virtual void onWarning(const std::string &warning, const HEVC::Parser::Info *pInfo, HEVC::Parser::WarningType);

    HEVC::Parser::Consumer         *m_pconsumer;

  protected:
    void sendWarning(const std::string &warning, const HEVC::Parser::Info *pInfo);
    std::string profileName(uint32_t);

    std::vector<uint32_t> columnWidthInLumaSamples(std::shared_ptr<HEVC::PPS> pPPS);
    std::vector<uint32_t> rowHeightInLumaSamples(std::shared_ptr<HEVC::PPS> pPPS);
    uint32_t ctbLog2SizeY(std::shared_ptr<HEVC::SPS> pSPS);

    void checkMainSPS(std::shared_ptr<HEVC::SPS> pSPS, const HEVC::Parser::Info *pInfo);
    void checkMain10SPS(std::shared_ptr<HEVC::SPS> pSPS, const HEVC::Parser::Info *pInfo);
    void checkMainStillPictureSPS(std::shared_ptr<HEVC::SPS> pSPS, const HEVC::Parser::Info *pInfo);

    void checkMainPPS(std::shared_ptr<HEVC::PPS> pPPS, const HEVC::Parser::Info *pInfo);
    void checkMain10PPS(std::shared_ptr<HEVC::PPS> pPPS, const HEVC::Parser::Info *pInfo);
    void checkMainStillPicturePPS(std::shared_ptr<HEVC::PPS> pPPS, const HEVC::Parser::Info *pInfo);


    uint32_t        m_profile;

    std::map<uint32_t, std::shared_ptr<HEVC::VPS> >          m_vpsMap;
    std::map<uint32_t, std::shared_ptr<HEVC::SPS> >          m_spsMap;
    std::map<uint32_t, std::shared_ptr<HEVC::PPS> >          m_ppsMap;

};


#endif
