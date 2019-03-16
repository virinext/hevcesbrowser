#ifndef COMMON_INFO_VIEWER_H_
#define COMMON_INFO_VIEWER_H_

#include <vector>
#include <map>

#include <QTableWidget>

#include <HevcParser.h>

#include <QMetaType>

#include "types.h"

class CommonInfoViewer:
  public QTableWidget,
  public HEVC::Parser::Consumer
{
  Q_OBJECT

  signals:
    void naluSelected(std::shared_ptr<HEVC::NALUnit>, ParserInfo info);
    void parameterSetsChanged(const VPSMap &vpsMap,
                          const SPSMap &spsMap,
                          const PPSMap &ppsMap);

  public:
    CommonInfoViewer(QWidget *pwgt = NULL);

    void saveCustomData();

    void clear();

    virtual void onNALUnit(std::shared_ptr<HEVC::NALUnit> pNALUnit, const HEVC::Parser::Info *pInfo);
    virtual void onWarning(const std::string &warning, const HEVC::Parser::Info *pInfo, HEVC::Parser::WarningType) {};

  private slots:
    void onSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

  private:
    void readCustomData();

    struct NALUInfo
    {
      std::shared_ptr<HEVC::NALUnit>     m_pNALUnit;
      HEVC::Parser::Info                 m_info;
    };

    std::vector<NALUInfo>              m_nalus;


    std::map<uint32_t, std::shared_ptr<HEVC::VPS> >          m_vpsMap;
    std::map<uint32_t, std::shared_ptr<HEVC::SPS> >          m_spsMap;
    std::map<uint32_t, std::shared_ptr<HEVC::PPS> >          m_ppsMap;
};


#endif
