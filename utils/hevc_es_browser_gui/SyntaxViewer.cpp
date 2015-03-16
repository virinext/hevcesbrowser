#include "SyntaxViewer.h"

#include <QStringList>
#include <QHeaderView>
#include <QString>

#include <QDebug>

#include <HevcUtils.h>

SyntaxViewer::SyntaxViewer(QWidget* pwgt):
  QTreeWidget(pwgt)
{
  header()->hide();
  QObject::connect(this, SIGNAL(itemCollapsed(QTreeWidgetItem *)), SLOT(onItemCollapsed(QTreeWidgetItem *)));
  QObject::connect(this, SIGNAL(itemExpanded(QTreeWidgetItem *)), SLOT(onItemExpanded(QTreeWidgetItem *)));
}


void SyntaxViewer::setParameretsSets(const VPSMap &vpsMap,
                      const SPSMap &spsMap,
                      const PPSMap &ppsMap)
{
  m_vpsMap = vpsMap;
  m_spsMap = spsMap;
  m_ppsMap = ppsMap;
}


void SyntaxViewer::onNalUChanged(std::shared_ptr<HEVC::NALUnit> pNalU)
{
  clear();
  using namespace HEVC;
  switch(pNalU -> m_nalUnitType)
  {
    case NAL_VPS:
    {
      std::shared_ptr<HEVC::VPS> pVPS = std::dynamic_pointer_cast<HEVC::VPS>(pNalU);
      createVPS(pVPS);
      break;
    }
    
    case NAL_SPS:
    {
      std::shared_ptr<HEVC::SPS> pSPS = std::dynamic_pointer_cast<HEVC::SPS>(pNalU);
      createSPS(pSPS);
      break;
    }
    
    case NAL_PPS:
    {
      std::shared_ptr<HEVC::PPS> pPPS = std::dynamic_pointer_cast<HEVC::PPS>(pNalU);
      createPPS(pPPS);
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
      createSlice(std::dynamic_pointer_cast<HEVC::Slice>(pNalU));
      break;
    }

    default:
      break;
  };

  updateItemsState();
}


void SyntaxViewer::createVPS(std::shared_ptr<HEVC::VPS> pVPS)
{
  QTreeWidgetItem *pvpsItem = new QTreeWidgetItem(QStringList("VPS"));
  addTopLevelItem(pvpsItem);

  pvpsItem -> addChild(new QTreeWidgetItem(QStringList("vps_video_parameter_set_id = " + QString::number(pVPS -> vps_video_parameter_set_id))));
  pvpsItem -> addChild(new QTreeWidgetItem(QStringList("vps_max_layers_minus1 = " + QString::number(pVPS -> vps_max_layers_minus1))));
  pvpsItem -> addChild(new QTreeWidgetItem(QStringList("vps_max_sub_layers_minus1 = " + QString::number(pVPS -> vps_max_sub_layers_minus1))));
  pvpsItem -> addChild(new QTreeWidgetItem(QStringList("vps_temporal_id_nesting_flag = " + QString::number(pVPS -> vps_temporal_id_nesting_flag))));
}



void SyntaxViewer::createSPS(std::shared_ptr<HEVC::SPS> pSPS)
{
  QTreeWidgetItem *pspsItem = new QTreeWidgetItem(QStringList("SPS"));
  addTopLevelItem(pspsItem);

  QTreeWidgetItem *pitem;
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("sps_video_parameter_set_id = " + QString::number(pSPS -> sps_video_parameter_set_id))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("sps_max_sub_layers_minus1 = " + QString::number(pSPS -> sps_max_sub_layers_minus1))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("sps_temporal_id_nesting_flag = " + QString::number(pSPS -> sps_temporal_id_nesting_flag))));

  pitem = new QTreeWidgetItem(QStringList("profile_tier_level"));
  pspsItem -> addChild(pitem);

  pitem -> addChild(new QTreeWidgetItem(QStringList("general_profile_space = " + QString::number(pSPS -> profile_tier_level.general_profile_space))));
  pitem -> addChild(new QTreeWidgetItem(QStringList("general_tier_flag = " + QString::number(pSPS -> profile_tier_level.general_tier_flag))));
  pitem -> addChild(new QTreeWidgetItem(QStringList("general_profile_idc = " + QString::number(pSPS -> profile_tier_level.general_profile_idc))));

  QString str = "general_profile_compatibility_flag[i] = { \n\t";
  for(std::size_t i=0; i<31; i++)
  {
    str += QString::number(pSPS -> profile_tier_level.general_profile_compatibility_flag[i]) + ", ";
    if((i+1) % 8 == 0)
      str += "\n\t";
  }
  str += QString::number(pSPS -> profile_tier_level.general_profile_compatibility_flag[31]) + "\n}";
  pitem -> addChild(new QTreeWidgetItem(QStringList(str)));



  pitem -> addChild(new QTreeWidgetItem(QStringList("general_progressive_source_flag = " + QString::number(pSPS -> profile_tier_level.general_progressive_source_flag))));
  pitem -> addChild(new QTreeWidgetItem(QStringList("general_interlaced_source_flag = " + QString::number(pSPS -> profile_tier_level.general_interlaced_source_flag))));
  pitem -> addChild(new QTreeWidgetItem(QStringList("general_non_packed_constraint_flag = " + QString::number(pSPS -> profile_tier_level.general_non_packed_constraint_flag))));
  pitem -> addChild(new QTreeWidgetItem(QStringList("general_frame_only_constraint_flag = " + QString::number(pSPS -> profile_tier_level.general_frame_only_constraint_flag))));
  pitem -> addChild(new QTreeWidgetItem(QStringList("general_level_idc = " + QString::number(pSPS -> profile_tier_level.general_level_idc))));

  if(pSPS -> sps_max_sub_layers_minus1 == 0)
  {
    pitem -> addChild(new QTreeWidgetItem(QStringList("sub_layer_profile_present_flag = {}")));
    pitem -> addChild(new QTreeWidgetItem(QStringList("sub_layer_level_present_flag = {}")));
  }
  else
  {
    str = "sub_layer_profile_present_flag = {\n\t";
    for(std::size_t i=0; i<pSPS -> sps_max_sub_layers_minus1 - 1; i++)
    {
      str += QString::number(pSPS -> profile_tier_level.sub_layer_profile_present_flag[i]) + ", ";
      if((i+1) % 8 == 0)
        str += "\n\t";
    }
    str += QString::number(pSPS -> profile_tier_level.sub_layer_profile_present_flag[pSPS -> sps_max_sub_layers_minus1 - 1]) + " \n}";
    pitem -> addChild(new QTreeWidgetItem(QStringList(str)));

    str = "sub_layer_level_present_flag = {\n\t";
    for(std::size_t i=0; i<pSPS -> sps_max_sub_layers_minus1 - 1; i++)
    {
      str += QString::number(pSPS -> profile_tier_level.sub_layer_level_present_flag[i]) + ", ";
      if((i+1) % 8 == 0)
        str += "\n\t";
    }
    str += QString::number(pSPS -> profile_tier_level.sub_layer_level_present_flag[pSPS -> sps_max_sub_layers_minus1 - 1]) + " \n}";
    pitem -> addChild(new QTreeWidgetItem(QStringList(str)));
  }



  pspsItem -> addChild(new QTreeWidgetItem(QStringList("sps_seq_parameter_set_id = " + QString::number(pSPS -> sps_seq_parameter_set_id))));
  pitem = new QTreeWidgetItem(QStringList("chroma_format_idc = " + QString::number(pSPS -> chroma_format_idc)));
  pspsItem -> addChild(pitem);
  if(pSPS -> chroma_format_idc == 3)
    pitem -> addChild(new QTreeWidgetItem(QStringList("separate_colour_plane_flag = " + QString::number(pSPS -> separate_colour_plane_flag))));


  pspsItem -> addChild(new QTreeWidgetItem(QStringList("pic_width_in_luma_samples = " + QString::number(pSPS -> pic_width_in_luma_samples))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("pic_height_in_luma_samples = " + QString::number(pSPS -> pic_height_in_luma_samples))));

  pitem = new QTreeWidgetItem(QStringList("conformance_window_flag = " + QString::number(pSPS -> conformance_window_flag)));
  pspsItem -> addChild(pitem);
  if(pSPS -> conformance_window_flag)
  {
    pitem -> addChild(new QTreeWidgetItem(QStringList("conf_win_left_offset = " + QString::number(pSPS -> conf_win_left_offset))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("conf_win_right_offset = " + QString::number(pSPS -> conf_win_right_offset))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("conf_win_top_offset = " + QString::number(pSPS -> conf_win_top_offset))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("conf_win_bottom_offset = " + QString::number(pSPS -> conf_win_bottom_offset))));
  }

  pspsItem -> addChild(new QTreeWidgetItem(QStringList("bit_depth_luma_minus8 = " + QString::number(pSPS -> bit_depth_luma_minus8))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("bit_depth_chroma_minus8 = " + QString::number(pSPS -> bit_depth_chroma_minus8))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("log2_max_pic_order_cnt_lsb_minus4 = " + QString::number(pSPS -> log2_max_pic_order_cnt_lsb_minus4))));

  pitem = new QTreeWidgetItem(QStringList("sps_sub_layer_ordering_info_present_flag = " + QString::number(pSPS -> sps_sub_layer_ordering_info_present_flag)));
  pspsItem -> addChild(pitem);

  str = "sps_max_dec_pic_buffering_minus1 = {\n\t";
  for(std::size_t i = (pSPS -> sps_sub_layer_ordering_info_present_flag ? 0 : pSPS -> sps_max_sub_layers_minus1); i <= pSPS -> sps_max_sub_layers_minus1; i++ ) 
  {
    str += QString::number(pSPS -> sps_max_dec_pic_buffering_minus1[i]) + ", ";
    if((i+1) % 8 == 0)
      str += "\n\t";
  }
  str += QString::number(pSPS -> sps_max_dec_pic_buffering_minus1[pSPS -> sps_max_sub_layers_minus1 - 1]) + " \n}";
  pitem -> addChild(new QTreeWidgetItem(QStringList(str)));

  str = "sps_max_num_reorder_pics = {\n\t";
  for(std::size_t i = (pSPS -> sps_sub_layer_ordering_info_present_flag ? 0 : pSPS -> sps_max_sub_layers_minus1); i <= pSPS -> sps_max_sub_layers_minus1; i++ ) 
  {
    str += QString::number(pSPS -> sps_max_num_reorder_pics[i]) + ", ";
    if((i+1) % 8 == 0)
      str += "\n\t";
  }
  str += QString::number(pSPS -> sps_max_num_reorder_pics[pSPS -> sps_max_sub_layers_minus1 - 1]) + " \n}";
  pitem -> addChild(new QTreeWidgetItem(QStringList(str)));

  str = "sps_max_latency_increase = {\n\t";
  for(std::size_t i = (pSPS -> sps_sub_layer_ordering_info_present_flag ? 0 : pSPS -> sps_max_sub_layers_minus1); i <= pSPS -> sps_max_sub_layers_minus1; i++ ) 
  {
    str += QString::number(pSPS -> sps_max_latency_increase[i]) + ", ";
    if((i+1) % 8 == 0)
      str += "\n\t";
  }
  str += QString::number(pSPS -> sps_max_latency_increase[pSPS -> sps_max_sub_layers_minus1 - 1]) + " \n}";
  pitem -> addChild(new QTreeWidgetItem(QStringList(str)));


  pspsItem -> addChild(new QTreeWidgetItem(QStringList("log2_min_luma_coding_block_size_minus3 = " + QString::number(pSPS -> log2_min_luma_coding_block_size_minus3))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("log2_diff_max_min_luma_coding_block_size = " + QString::number(pSPS -> log2_diff_max_min_luma_coding_block_size))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("log2_min_transform_block_size_minus2 = " + QString::number(pSPS -> log2_min_transform_block_size_minus2))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("log2_diff_max_min_transform_block_size = " + QString::number(pSPS -> log2_diff_max_min_transform_block_size))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("max_transform_hierarchy_depth_inter = " + QString::number(pSPS -> max_transform_hierarchy_depth_inter))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("max_transform_hierarchy_depth_intra = " + QString::number(pSPS -> max_transform_hierarchy_depth_intra))));

  pitem = new QTreeWidgetItem(QStringList("scaling_list_enabled_flag = " + QString::number(pSPS -> scaling_list_enabled_flag)));
  pspsItem -> addChild(pitem);

  if(pSPS -> scaling_list_enabled_flag)
  {
    pitem -> addChild(new QTreeWidgetItem(QStringList("sps_scaling_list_data_present_flag = " + QString::number(pSPS -> sps_scaling_list_data_present_flag))));
    //scaling_list_data( )
  }

  pspsItem -> addChild(new QTreeWidgetItem(QStringList("amp_enabled_flag = " + QString::number(pSPS -> amp_enabled_flag))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("sample_adaptive_offset_enabled_flag = " + QString::number(pSPS -> sample_adaptive_offset_enabled_flag))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("pcm_enabled_flag = " + QString::number(pSPS -> pcm_enabled_flag))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("num_short_term_ref_pic_sets = " + QString::number(pSPS -> num_short_term_ref_pic_sets))));
}

void SyntaxViewer::createPPS(std::shared_ptr<HEVC::PPS> pPPS)
{
  QTreeWidgetItem *pppsItem = new QTreeWidgetItem(QStringList("PPS"));
  addTopLevelItem(pppsItem);

  QTreeWidgetItem *pitem;

  pppsItem -> addChild(new QTreeWidgetItem(QStringList("pps_pic_parameter_set_id = " + QString::number(pPPS -> pps_pic_parameter_set_id))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("pps_seq_parameter_set_id = " + QString::number(pPPS -> pps_seq_parameter_set_id))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("dependent_slice_segments_enabled_flag = " + QString::number(pPPS -> dependent_slice_segments_enabled_flag))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("output_flag_present_flag = " + QString::number(pPPS -> output_flag_present_flag))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("num_extra_slice_header_bits = " + QString::number(pPPS -> num_extra_slice_header_bits))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("sign_data_hiding_flag = " + QString::number(pPPS -> sign_data_hiding_flag))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("cabac_init_present_flag = " + QString::number(pPPS -> cabac_init_present_flag))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("num_ref_idx_l0_default_active_minus1 = " + QString::number(pPPS -> num_ref_idx_l0_default_active_minus1))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("num_ref_idx_l1_default_active_minus1 = " + QString::number(pPPS -> num_ref_idx_l1_default_active_minus1))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("init_qp_minus26 = " + QString::number(pPPS -> init_qp_minus26))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("constrained_intra_pred_flag = " + QString::number(pPPS -> constrained_intra_pred_flag))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("transform_skip_enabled_flag = " + QString::number(pPPS -> transform_skip_enabled_flag))));

  pitem = new QTreeWidgetItem(QStringList("cu_qp_delta_enabled_flag = " + QString::number(pPPS -> cu_qp_delta_enabled_flag)));
  pppsItem -> addChild(pitem);

  if(pPPS -> cu_qp_delta_enabled_flag)
    pitem -> addChild(new QTreeWidgetItem(QStringList("diff_cu_qp_delta_depth = " + QString::number(pPPS -> diff_cu_qp_delta_depth))));


  pppsItem -> addChild(new QTreeWidgetItem(QStringList("pps_cb_qp_offset = " + QString::number(pPPS -> pps_cb_qp_offset))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("pps_cr_qp_offset = " + QString::number(pPPS -> pps_cr_qp_offset))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("pps_slice_chroma_qp_offsets_present_flag = " + QString::number(pPPS -> pps_slice_chroma_qp_offsets_present_flag))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("weighted_pred_flag = " + QString::number(pPPS -> weighted_pred_flag))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("weighted_bipred_flag = " + QString::number(pPPS -> weighted_bipred_flag))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("transquant_bypass_enabled_flag = " + QString::number(pPPS -> transquant_bypass_enabled_flag))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("tiles_enabled_flag = " + QString::number(pPPS -> tiles_enabled_flag))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("entropy_coding_sync_enabled_flag = " + QString::number(pPPS -> entropy_coding_sync_enabled_flag))));
}

void SyntaxViewer::createSlice(std::shared_ptr<HEVC::Slice> pSlice)
{
  std::shared_ptr<HEVC::PPS> pPPS = m_ppsMap[pSlice -> slice_pic_parameter_set_id];
  int32_t spsId = pPPS -> pps_seq_parameter_set_id;

  QTreeWidgetItem *psliceItem = new QTreeWidgetItem(QStringList("Slice"));
  addTopLevelItem(psliceItem);

  QTreeWidgetItem *pitem;

  psliceItem -> addChild(new QTreeWidgetItem(QStringList("first_slice_segment_in_pic_flag = " + QString::number(pSlice -> first_slice_segment_in_pic_flag))));


  if(pSlice -> m_nalUnitType >= HEVC::NAL_BLA_W_LP && pSlice -> m_nalUnitType <= HEVC::NAL_IRAP_VCL23)
  {
    pitem = new QTreeWidgetItem(QStringList("nalUnitType >= HEVC::NAL_BLA_W_LP && nalUnitType <= HEVC::NAL_IRAP_VCL23"));
    psliceItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("no_output_of_prior_pics_flag = " + QString::number(pSlice -> no_output_of_prior_pics_flag))));
  }

  psliceItem -> addChild(new QTreeWidgetItem(QStringList("slice_pic_parameter_set_id = " + QString::number(pSlice -> slice_pic_parameter_set_id))));
  if(!pSlice -> first_slice_segment_in_pic_flag)
  {
    pitem = new QTreeWidgetItem(QStringList("first_slice_segment_in_pic_flag == 1"));
    psliceItem -> addChild(pitem);

    QTreeWidgetItem *pitemDepend = new QTreeWidgetItem(QStringList("pps -> dependent_slice_segments_enabled_flag = " + QString::number(pPPS -> dependent_slice_segments_enabled_flag)));

    pitem -> addChild(pitemDepend);

    if(pPPS -> dependent_slice_segments_enabled_flag)
      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("dependent_slice_segment_flag = " + QString::number(pSlice -> dependent_slice_segment_flag))));

    pitem -> addChild(new QTreeWidgetItem(QStringList("slice_segment_address = " + QString::number(pSlice -> slice_segment_address))));
  }
  if(!pSlice -> dependent_slice_segment_flag)
  {
    pitem = new QTreeWidgetItem(QStringList("dependent_slice_segment_flag == 1"));
    psliceItem -> addChild(pitem);

    long num_extra_slice_header_bits = pPPS -> num_extra_slice_header_bits;

    QString str = "slice_reserved_undetermined_flag = {\n\t";

    if(num_extra_slice_header_bits > 0)
    {
      for(long i=0; i<num_extra_slice_header_bits - 1; i++)
        str += QString::number(pSlice -> slice_reserved_undetermined_flag[i]) + ", ";
      str += QString::number(pSlice -> slice_reserved_undetermined_flag[num_extra_slice_header_bits - 1]) + ", ";
    }

    pitem -> addChild(new QTreeWidgetItem(QStringList(str)));
    pitem -> addChild(new QTreeWidgetItem(QStringList("slice_type = " + QString::number(pSlice -> slice_type))));
    QTreeWidgetItem *pitemDepend = new QTreeWidgetItem(QStringList("pps -> output_flag_present_flag = " + QString::number(pPPS -> output_flag_present_flag)));
    pitem -> addChild(pitemDepend);

    if(pPPS -> output_flag_present_flag)
      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("pic_output_flag = " + QString::number(pSlice -> pic_output_flag))));

    pitemDepend = new QTreeWidgetItem(QStringList("sps -> separate_colour_plane_flag = " + QString::number(m_spsMap[spsId] -> separate_colour_plane_flag)));
    pitem -> addChild(pitemDepend);

    if(m_spsMap[spsId] -> separate_colour_plane_flag)
      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("colour_plane_id = " + QString::number(pSlice -> colour_plane_id))));
    
    bool IdrPicFlag = pSlice -> m_nalUnitType == HEVC::NAL_IDR_W_RADL || pSlice -> m_nalUnitType == HEVC::NAL_IDR_N_LP;
    if(!IdrPicFlag)
    {
      pitemDepend = new QTreeWidgetItem(QStringList("nalUnitType != NAL_IDR_W_RADL && nalUnitType != NAL_IDR_N_LP"));
      pitem -> addChild(pitemDepend);

      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("pic_order_cnt_lsb = " + QString::number(pSlice -> pic_order_cnt_lsb))));
      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("short_term_ref_pic_set_sps_flag = " + QString::number(pSlice -> short_term_ref_pic_set_sps_flag))));

      pitem = new QTreeWidgetItem(QStringList("short_term_ref_pic_set_sps_flag = " + QString::number(pSlice -> short_term_ref_pic_set_sps_flag)));
      pitemDepend -> addChild(pitem);
      if(!pSlice -> short_term_ref_pic_set_sps_flag)
      {
        //short_term_ref_pic_set
      }
      else if(pSlice -> short_term_ref_pic_set_sps_flag > 1)
      {
        std::size_t numBits = HEVC::log2(m_spsMap[spsId] -> num_short_term_ref_pic_sets);
        pitemDepend = new QTreeWidgetItem(QStringList("log2(sps -> num_short_term_ref_pic_sets) = " + QString::number(numBits)));
        pitem -> addChild(pitemDepend);
        if(numBits > 0)
          pitemDepend -> addChild(new QTreeWidgetItem(QStringList("short_term_ref_pic_set_idx = " + QString::number(pSlice -> short_term_ref_pic_set_sps_flag))));
      }
    }
  }
}

void SyntaxViewer::onItemCollapsed(QTreeWidgetItem * item)
{
  QString text = item -> text(0);

  if(text.startsWith("SPS"))
    m_state.m_sps.m_sps = false;
  else if(text.startsWith("profile_tier_level"))
    m_state.m_sps.m_profile_tier_level = false;
  else if(text.startsWith("chroma_format_idc"))
    m_state.m_sps.m_chroma_format_idc = false;
  else if(text.startsWith("conformance_window_flag"))
    m_state.m_sps.m_conformance_window_flag = false;
  else if(text.startsWith("sps_sub_layer_ordering_info_present_flag"))
    m_state.m_sps.m_sps_sub_layer_ordering_info_present_flag = false;
  else if(text.startsWith("scaling_list_enabled_flag"))
    m_state.m_sps.m_scaling_list_enabled_flag = false;
  else if(text.startsWith("VPS"))
    m_state.m_vps.m_vps = false;
  else if(text.startsWith("PPS"))
    m_state.m_pps.m_pps = false;
  else if(text.startsWith("cu_qp_delta_enabled_flag"))
    m_state.m_pps.m_cu_qp_delta_enabled_flag = false;
  else if(text.startsWith("Slice"))
    m_state.m_slice.m_slice = false;
  else if(text.startsWith("nalUnitType >= HEVC::NAL_BLA_W_LP && nalUnitType <= HEVC::NAL_IRAP_VCL23"))
    m_state.m_slice.m_nalUnitType = false;
  else if(text.startsWith("m_first_slice_segment_in_pic_flag"))
    m_state.m_slice.m_first_slice_segment_in_pic_flag = false;
  else if(text.startsWith("pps -> dependent_slice_segments_enabled_flag"))
    m_state.m_slice.m_dependent_slice_segments_enabled_flag = false;
  else if(text.startsWith("dependent_slice_segment_flag == 1"))
    m_state.m_slice.m_dependent_slice_segment_flag = false;
  else if(text.startsWith("pps -> output_flag_present_flag"))
    m_state.m_slice.m_output_flag_present_flag = false;
  else if(text.startsWith("sps -> separate_colour_plane_flag"))
    m_state.m_slice.m_separate_colour_plane_flag = false;
  else if(text.startsWith("nalUnitType != NAL_IDR_W_RADL && nalUnitType != NAL_IDR_N_LP"))
    m_state.m_slice.m_nalUnitTypeIsNotIdr = false;
  else if(text.startsWith("short_term_ref_pic_set_sps_flag"))
    m_state.m_slice.m_short_term_ref_pic_set_sps_flag = false;
  else if(text.startsWith("log2(sps -> num_short_term_ref_pic_sets)"))
    m_state.m_slice.m_log2_sps_num_short_term_ref_pic_sets = false;

}



void SyntaxViewer::onItemExpanded(QTreeWidgetItem * item)
{
  QString text = item -> text(0);

  if(text.startsWith("SPS"))
    m_state.m_sps.m_sps = true;
  else if(text.startsWith("profile_tier_level"))
    m_state.m_sps.m_profile_tier_level = true;
  else if(text.startsWith("chroma_format_idc"))
    m_state.m_sps.m_chroma_format_idc = true;
  else if(text.startsWith("conformance_window_flag"))
    m_state.m_sps.m_conformance_window_flag = true;
  else if(text.startsWith("sps_sub_layer_ordering_info_present_flag"))
    m_state.m_sps.m_sps_sub_layer_ordering_info_present_flag = true;
  else if(text.startsWith("scaling_list_enabled_flag"))
    m_state.m_sps.m_scaling_list_enabled_flag = true;
  else if(text.startsWith("VPS"))
    m_state.m_vps.m_vps = true;
  else if(text.startsWith("PPS"))
    m_state.m_pps.m_pps = true;
  else if(text.startsWith("cu_qp_delta_enabled_flag"))
    m_state.m_pps.m_cu_qp_delta_enabled_flag = true;
  else if(text.startsWith("Slice"))
    m_state.m_slice.m_slice = true;
  else if(text.startsWith("nalUnitType >= HEVC::NAL_BLA_W_LP && nalUnitType <= HEVC::NAL_IRAP_VCL23"))
    m_state.m_slice.m_nalUnitType = true;
  else if(text.startsWith("m_first_slice_segment_in_pic_flag"))
    m_state.m_slice.m_first_slice_segment_in_pic_flag = true;
  else if(text.startsWith("pps -> dependent_slice_segments_enabled_flag"))
    m_state.m_slice.m_dependent_slice_segments_enabled_flag = true;
  else if(text.startsWith("dependent_slice_segment_flag == 1"))
    m_state.m_slice.m_dependent_slice_segment_flag = true;
  else if(text.startsWith("pps -> output_flag_present_flag"))
    m_state.m_slice.m_output_flag_present_flag = true;
  else if(text.startsWith("sps -> separate_colour_plane_flag"))
    m_state.m_slice.m_separate_colour_plane_flag = true;
  else if(text.startsWith("nalUnitType != NAL_IDR_W_RADL && nalUnitType != NAL_IDR_N_LP"))
    m_state.m_slice.m_nalUnitTypeIsNotIdr = true;
  else if(text.startsWith("short_term_ref_pic_set_sps_flag"))
    m_state.m_slice.m_short_term_ref_pic_set_sps_flag = true;
  else if(text.startsWith("log2(sps -> num_short_term_ref_pic_sets)"))
    m_state.m_slice.m_log2_sps_num_short_term_ref_pic_sets = true;

}



void SyntaxViewer::updateItemsState()
{
  QTreeWidgetItemIterator itr(this);
  for(;*itr; itr++) 
  {
    QString text = (*itr) -> text(0);

    if(text.startsWith("SPS"))
      (*itr)-> setExpanded(m_state.m_sps.m_sps);
    else if(text.startsWith("profile_tier_level"))
      (*itr)-> setExpanded(m_state.m_sps.m_profile_tier_level);
    else if(text.startsWith("chroma_format_idc"))
      (*itr)-> setExpanded(m_state.m_sps.m_chroma_format_idc);
    else if(text.startsWith("conformance_window_flag"))
      (*itr)-> setExpanded(m_state.m_sps.m_conformance_window_flag);
    else if(text.startsWith("sps_sub_layer_ordering_info_present_flag"))
      (*itr)-> setExpanded(m_state.m_sps.m_sps_sub_layer_ordering_info_present_flag);
    else if(text.startsWith("scaling_list_enabled_flag"))
      (*itr)-> setExpanded(m_state.m_sps.m_scaling_list_enabled_flag);
    else if(text.startsWith("VPS"))
      (*itr)-> setExpanded(m_state.m_vps.m_vps);
    else if(text.startsWith("PPS"))
      (*itr)-> setExpanded(m_state.m_pps.m_pps);
    else if(text.startsWith("cu_qp_delta_enabled_flag"))
      (*itr)-> setExpanded(m_state.m_pps.m_cu_qp_delta_enabled_flag);
    else if(text.startsWith("Slice"))
      (*itr)-> setExpanded(m_state.m_slice.m_slice);
    else if(text.startsWith("nalUnitType >= HEVC::NAL_BLA_W_LP && nalUnitType <= HEVC::NAL_IRAP_VCL23"))
      (*itr)-> setExpanded(m_state.m_slice.m_nalUnitType);
    else if(text.startsWith("m_first_slice_segment_in_pic_flag"))
      (*itr)-> setExpanded(m_state.m_slice.m_first_slice_segment_in_pic_flag);
    else if(text.startsWith("pps -> dependent_slice_segments_enabled_flag"))
      (*itr)-> setExpanded(m_state.m_slice.m_dependent_slice_segments_enabled_flag);
    else if(text.startsWith("dependent_slice_segment_flag == 1"))
      (*itr)-> setExpanded(m_state.m_slice.m_dependent_slice_segment_flag);
    else if(text.startsWith("pps -> output_flag_present_flag"))
      (*itr)-> setExpanded(m_state.m_slice.m_output_flag_present_flag);
    else if(text.startsWith("sps -> separate_colour_plane_flag"))
      (*itr)-> setExpanded(m_state.m_slice.m_separate_colour_plane_flag);
    else if(text.startsWith("nalUnitType != NAL_IDR_W_RADL && nalUnitType != NAL_IDR_N_LP"))
      (*itr)-> setExpanded(m_state.m_slice.m_nalUnitTypeIsNotIdr);
    else if(text.startsWith("short_term_ref_pic_set_sps_flag"))
      (*itr)-> setExpanded(m_state.m_slice.m_short_term_ref_pic_set_sps_flag);
    else if(text.startsWith("log2(sps -> num_short_term_ref_pic_sets)"))
      (*itr)-> setExpanded(m_state.m_slice.m_log2_sps_num_short_term_ref_pic_sets);
  }
}
