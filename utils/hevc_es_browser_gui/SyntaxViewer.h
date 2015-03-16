#ifndef SYNTAX_VIEWER_H_
#define SYNTAX_VIEWER_H_



#include <QTreeWidget>

#include <Hevc.h>

#include <map>
#include <memory>

#include "types.h"

class SyntaxViewer: public QTreeWidget
{
  Q_OBJECT
  
  public:
    SyntaxViewer(QWidget *pwgt = NULL);

    void updateItemsState();
    
  public slots:
    void onNalUChanged(std::shared_ptr<HEVC::NALUnit>);

    void setParameretsSets(const VPSMap &vpsMap,
                      const SPSMap &spsMap,
                      const PPSMap &ppsMap);

  private slots:
    void onItemCollapsed(QTreeWidgetItem * item);
    void onItemExpanded(QTreeWidgetItem * item);

private:
    void createVPS(std::shared_ptr<HEVC::VPS> pVPS);
    void createSPS(std::shared_ptr<HEVC::SPS> pSPS);
    void createPPS(std::shared_ptr<HEVC::PPS> pPPS);
    void createSlice(std::shared_ptr<HEVC::Slice> pSlice);


    std::map<uint32_t, std::shared_ptr<HEVC::VPS> >          m_vpsMap;
    std::map<uint32_t, std::shared_ptr<HEVC::SPS> >          m_spsMap;
    std::map<uint32_t, std::shared_ptr<HEVC::PPS> >          m_ppsMap;

    struct SyntaxViewerState
    {
      struct SPS
      {

        SPS():
          m_sps(true)
          ,m_profile_tier_level(true)
          ,m_chroma_format_idc(true)
          ,m_conformance_window_flag(true)
          ,m_sps_sub_layer_ordering_info_present_flag(true)
          ,m_scaling_list_enabled_flag(true)
        {
        }

        bool   m_sps;
        bool   m_profile_tier_level;
        bool   m_chroma_format_idc;
        bool   m_conformance_window_flag;
        bool   m_sps_sub_layer_ordering_info_present_flag;
        bool   m_scaling_list_enabled_flag;
      };

      struct VPS
      {
        VPS():
          m_vps(true)
        {
        }

        bool   m_vps;

      };

      struct PPS
      {
        PPS():
          m_pps(true)
          ,m_cu_qp_delta_enabled_flag(true)
        {
        }

        bool   m_pps;
        bool   m_cu_qp_delta_enabled_flag;
      };

      struct Slice
      {
        Slice():
          m_slice(true)
          ,m_nalUnitType(true)
          ,m_first_slice_segment_in_pic_flag(true)
          ,m_dependent_slice_segments_enabled_flag(true)
          ,m_dependent_slice_segment_flag(true)
          ,m_output_flag_present_flag(true)
          ,m_separate_colour_plane_flag(true)
          ,m_nalUnitTypeIsNotIdr(true)
          ,m_short_term_ref_pic_set_sps_flag(true)
          ,m_log2_sps_num_short_term_ref_pic_sets(true)
        {
        }

        bool   m_slice;
        bool   m_nalUnitType;
        bool   m_first_slice_segment_in_pic_flag;
        bool   m_dependent_slice_segments_enabled_flag;
        bool   m_dependent_slice_segment_flag;
        bool   m_output_flag_present_flag;
        bool   m_separate_colour_plane_flag;
        bool   m_nalUnitTypeIsNotIdr;
        bool   m_short_term_ref_pic_set_sps_flag;
        bool   m_log2_sps_num_short_term_ref_pic_sets;
      };

      VPS     m_vps;
      SPS     m_sps;
      PPS     m_pps;
      Slice   m_slice;
    };
    
    
    SyntaxViewerState     m_state;
};



#endif
