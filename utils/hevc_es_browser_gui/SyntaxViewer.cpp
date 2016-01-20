#include "SyntaxViewer.h"

#include <QStringList>
#include <QHeaderView>
#include <QString>

#include <QDebug>

#include <HevcUtils.h>

#define SLICE_B 0
#define SLICE_P 1
#define SLICE_I 2

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


void SyntaxViewer::onNalUChanged(std::shared_ptr<HEVC::NALUnit> pNalU, ParserInfo info)
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

    case NAL_AUD:
    {
      createAUD(std::dynamic_pointer_cast<HEVC::AUD>(pNalU));
      break;
    }

    case NAL_SEI_PREFIX:
    case NAL_SEI_SUFFIX:
    {
      createSEI(std::dynamic_pointer_cast<HEVC::SEI>(pNalU));
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

  QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("profile_tier_level"));
  pvpsItem -> addChild(pitem);
  createProfileTierLevel(pVPS -> profile_tier_level, pitem);


  pvpsItem -> addChild(new QTreeWidgetItem(QStringList("vps_sub_layer_ordering_info_present_flag = " + QString::number(pVPS -> vps_sub_layer_ordering_info_present_flag))));

  QTreeWidgetItem *ploop = new QTreeWidgetItem(QStringList("for( i = ( vps_sub_layer_ordering_info_present_flag ? 0 : vps_max_sub_layers_minus1 ); i <= vps_max_sub_layers_minus1; i++ )"));
  
  pvpsItem -> addChild(ploop);

  for(std::size_t i = (pVPS -> vps_sub_layer_ordering_info_present_flag ? 0 : pVPS -> vps_max_sub_layers_minus1); i <= pVPS -> vps_max_sub_layers_minus1; i++)
  {
    ploop -> addChild(new QTreeWidgetItem(QStringList("vps_max_dec_pic_buffering_minus1[" + QString::number(i) + "] = " + QString::number(pVPS -> vps_max_dec_pic_buffering_minus1[i]))));
    ploop -> addChild(new QTreeWidgetItem(QStringList("vps_max_num_reorder_pics[" + QString::number(i) + "] = " + QString::number(pVPS -> vps_max_num_reorder_pics[i]))));
    ploop -> addChild(new QTreeWidgetItem(QStringList("vps_max_latency_increase_plus1[" + QString::number(i) + "] = " + QString::number(pVPS -> vps_max_latency_increase_plus1[i]))));
  }

  pvpsItem -> addChild(new QTreeWidgetItem(QStringList("vps_max_layer_id = " + QString::number(pVPS -> vps_max_layer_id))));
  pvpsItem -> addChild(new QTreeWidgetItem(QStringList("vps_num_layer_sets_minus1 = " + QString::number(pVPS -> vps_num_layer_sets_minus1))));


  if(pVPS -> vps_num_layer_sets_minus1 == 0)
  {
      pvpsItem -> addChild(new QTreeWidgetItem(QStringList("layer_id_included_flag = { }")));
  }
  else
  {
    QTreeWidgetItem *ploop = new QTreeWidgetItem(QStringList("for(std::size_t i=0; i<vps_num_layer_sets_minus1; i++)"));

    for(std::size_t i=0; i<pVPS -> vps_num_layer_sets_minus1; i++)
    {
      QString str;

      if(pVPS -> vps_max_layer_id == 0)
        str = "layer_id_included_flag[" + QString::number(i) + "] = { } ";
      else
      {
        for(int j=0; j<pVPS -> vps_max_layer_id - 1; j++)
        {
          str += QString::number(pVPS -> layer_id_included_flag[i][j]) + ",  ";
          if((j + 1) % 8)
            str += "\n";
        }
        str += QString::number(pVPS -> layer_id_included_flag[i][pVPS -> vps_max_layer_id - 1]) + " } ";
      }
      ploop -> addChild(new QTreeWidgetItem(QStringList(str)));
    }
  }

  pvpsItem -> addChild(new QTreeWidgetItem(QStringList("vps_timing_info_present_flag = " + QString::number(pVPS -> vps_timing_info_present_flag))));

  if(pVPS -> vps_timing_info_present_flag) 
  {
    QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if( vps_timing_info_present_flag )"));
    pvpsItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("vps_num_units_in_tick = " + QString::number(pVPS -> vps_num_units_in_tick))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("vps_time_scale = " + QString::number(pVPS -> vps_time_scale))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("vps_poc_proportional_to_timing_flag = " + QString::number(pVPS -> vps_poc_proportional_to_timing_flag))));

    if(pVPS -> vps_poc_proportional_to_timing_flag)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( vps_poc_proportional_to_timing_flag )"));
      pitem -> addChild(pitemSecond);
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("vps_num_ticks_poc_diff_one_minus1 = " + QString::number(pVPS -> vps_num_ticks_poc_diff_one_minus1))));
    }

    pitem -> addChild(new QTreeWidgetItem(QStringList("vps_num_hrd_parameters = " + QString::number(pVPS -> vps_num_hrd_parameters))));

    if(pVPS -> vps_num_hrd_parameters)
    {
      QTreeWidgetItem *ploop = new QTreeWidgetItem(QStringList("for( i = 0; i < vps_num_hrd_parameters; i++ )"));
      pitem -> addChild(ploop);

      for(std::size_t i=0; i<pVPS -> vps_num_hrd_parameters; i++)
      {
        ploop -> addChild(new QTreeWidgetItem(QStringList("hrd_layer_set_idx[ " + QString::number(i) + "] = " + QString::number(pVPS -> hrd_layer_set_idx[i]))));
        if(i > 0)
        {
          QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( i > 0 )"));
          ploop -> addChild(pitemSecond);

          pitemSecond -> addChild(new QTreeWidgetItem(QStringList("cprms_present_flag[ " + QString::number(i) + "] = " + QString::number(pVPS -> cprms_present_flag[i]))));
        }


        QTreeWidgetItem *pitemThird = new QTreeWidgetItem(QStringList("hrd_parameters(" + QString::number(i) + ", " + QString::number(pVPS -> vps_max_sub_layers_minus1) + ")"));
        ploop -> addChild(pitemThird);

        createHrdParameters(pVPS -> hrd_parameters[i], i, pitemThird);
      }
    }
  }

  pvpsItem -> addChild(new QTreeWidgetItem(QStringList("vps_extension_flag = " + QString::number(pVPS -> vps_extension_flag))));
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
  createProfileTierLevel(pSPS -> profile_tier_level, pitem);



  pspsItem -> addChild(new QTreeWidgetItem(QStringList("sps_seq_parameter_set_id = " + QString::number(pSPS -> sps_seq_parameter_set_id))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("chroma_format_idc = " + QString::number(pSPS -> chroma_format_idc))));

  if(pSPS -> chroma_format_idc == 3)
  {
    pitem = new QTreeWidgetItem(QStringList("if( chroma_format_idc == 3 )"));
    pspsItem -> addChild(pitem);
    pitem -> addChild(new QTreeWidgetItem(QStringList("separate_colour_plane_flag = " + QString::number(pSPS -> separate_colour_plane_flag))));
  }


  pspsItem -> addChild(new QTreeWidgetItem(QStringList("pic_width_in_luma_samples = " + QString::number(pSPS -> pic_width_in_luma_samples))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("pic_height_in_luma_samples = " + QString::number(pSPS -> pic_height_in_luma_samples))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("conformance_window_flag = " + QString::number(pSPS -> conformance_window_flag))));

  if(pSPS -> conformance_window_flag)
  {
    pitem = new QTreeWidgetItem(QStringList("if( conformance_window_flag )"));
    pspsItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("conf_win_left_offset = " + QString::number(pSPS -> conf_win_left_offset))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("conf_win_right_offset = " + QString::number(pSPS -> conf_win_right_offset))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("conf_win_top_offset = " + QString::number(pSPS -> conf_win_top_offset))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("conf_win_bottom_offset = " + QString::number(pSPS -> conf_win_bottom_offset))));
  }

  pspsItem -> addChild(new QTreeWidgetItem(QStringList("bit_depth_luma_minus8 = " + QString::number(pSPS -> bit_depth_luma_minus8))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("bit_depth_chroma_minus8 = " + QString::number(pSPS -> bit_depth_chroma_minus8))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("log2_max_pic_order_cnt_lsb_minus4 = " + QString::number(pSPS -> log2_max_pic_order_cnt_lsb_minus4))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("sps_sub_layer_ordering_info_present_flag = " + QString::number(pSPS -> sps_sub_layer_ordering_info_present_flag))));

  pitem = new QTreeWidgetItem(QStringList("for( i = ( sps_sub_layer_ordering_info_present_flag ? 0 : sps_max_sub_layers_minus1 );i <= sps_max_sub_layers_minus1; i++ )"));
  pspsItem -> addChild(pitem);

  for(std::size_t i = (pSPS -> sps_sub_layer_ordering_info_present_flag ? 0 : pSPS -> sps_max_sub_layers_minus1); i <= pSPS -> sps_max_sub_layers_minus1; i++ ) 
  {
    pitem -> addChild(new QTreeWidgetItem(QStringList("sps_max_dec_pic_buffering_minus1[" + QString::number(i) + "] = " + QString::number(pSPS -> sps_max_dec_pic_buffering_minus1[i]))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("sps_max_num_reorder_pics[" + QString::number(i) + "] = " + QString::number(pSPS -> sps_max_num_reorder_pics[i]))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("sps_max_latency_increase_plus1[" + QString::number(i) + "] = " + QString::number(pSPS -> sps_max_latency_increase_plus1[i]))));
  }


  pspsItem -> addChild(new QTreeWidgetItem(QStringList("log2_min_luma_coding_block_size_minus3 = " + QString::number(pSPS -> log2_min_luma_coding_block_size_minus3))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("log2_diff_max_min_luma_coding_block_size = " + QString::number(pSPS -> log2_diff_max_min_luma_coding_block_size))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("log2_min_transform_block_size_minus2 = " + QString::number(pSPS -> log2_min_transform_block_size_minus2))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("log2_diff_max_min_transform_block_size = " + QString::number(pSPS -> log2_diff_max_min_transform_block_size))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("max_transform_hierarchy_depth_inter = " + QString::number(pSPS -> max_transform_hierarchy_depth_inter))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("max_transform_hierarchy_depth_intra = " + QString::number(pSPS -> max_transform_hierarchy_depth_intra))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("scaling_list_enabled_flag = " + QString::number(pSPS -> scaling_list_enabled_flag))));

  if(pSPS -> scaling_list_enabled_flag)
  {
    pitem = new QTreeWidgetItem(QStringList("if( scaling_list_enabled_flag )"));
    pspsItem -> addChild(pitem);
    pitem -> addChild(new QTreeWidgetItem(QStringList("sps_scaling_list_data_present_flag = " + QString::number(pSPS -> sps_scaling_list_data_present_flag))));

    if(pSPS -> sps_scaling_list_data_present_flag)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("scaling_list_data( )"));
      pitem -> addChild(pitemSecond);
      createScalingListData(pSPS -> scaling_list_data, pitemSecond);
    }
  }

  pspsItem -> addChild(new QTreeWidgetItem(QStringList("amp_enabled_flag = " + QString::number(pSPS -> amp_enabled_flag))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("sample_adaptive_offset_enabled_flag = " + QString::number(pSPS -> sample_adaptive_offset_enabled_flag))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("pcm_enabled_flag = " + QString::number(pSPS -> pcm_enabled_flag))));
  if(pSPS -> pcm_enabled_flag)
  {
    pitem = new QTreeWidgetItem(QStringList("if( pcm_enabled_flag )"));
    pspsItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("pcm_sample_bit_depth_luma_minus1 = " + QString::number(pSPS -> pcm_sample_bit_depth_luma_minus1))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("pcm_sample_bit_depth_chroma_minus1 = " + QString::number(pSPS -> pcm_sample_bit_depth_chroma_minus1))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("log2_min_pcm_luma_coding_block_size_minus3 = " + QString::number(pSPS -> log2_min_pcm_luma_coding_block_size_minus3))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("log2_diff_max_min_pcm_luma_coding_block_size = " + QString::number(pSPS -> log2_diff_max_min_pcm_luma_coding_block_size))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("pcm_loop_filter_disabled_flag = " + QString::number(pSPS -> pcm_loop_filter_disabled_flag))));
  }

  pspsItem -> addChild(new QTreeWidgetItem(QStringList("num_short_term_ref_pic_sets = " + QString::number(pSPS -> num_short_term_ref_pic_sets))));

  if(pSPS -> num_short_term_ref_pic_sets)
  {
    pitem = new QTreeWidgetItem(QStringList("for( i = 0; i < num_short_term_ref_pic_sets; i++)"));
    pspsItem -> addChild(pitem);

    for(std::size_t i=0; i<pSPS -> num_short_term_ref_pic_sets; i++)
    {
      QTreeWidgetItem *pStrpc = new QTreeWidgetItem(QStringList("short_term_ref_pic_set(" + QString::number(i) + ")"));
      pitem -> addChild(pStrpc);

      HEVC::ShortTermRefPicSet rpset = pSPS -> short_term_ref_pic_set[i];
      createShortTermRefPicSet(i, rpset, pSPS -> num_short_term_ref_pic_sets, pSPS -> short_term_ref_pic_set, pStrpc);
    }
  }


  pspsItem -> addChild(new QTreeWidgetItem(QStringList("long_term_ref_pics_present_flag = " + QString::number(pSPS -> long_term_ref_pics_present_flag))));
  if(pSPS -> long_term_ref_pics_present_flag)
  {
    pitem = new QTreeWidgetItem(QStringList("if( long_term_ref_pics_present_flag )"));
    pspsItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("num_long_term_ref_pics_sps = " + QString::number(pSPS -> num_long_term_ref_pics_sps))));


    if(pSPS -> num_long_term_ref_pics_sps > 0)
    {
      QTreeWidgetItem *pitemLoop = new QTreeWidgetItem(QStringList("for( i = 0; i < num_long_term_ref_pics_sps; i++ )"));
      pitem -> addChild(pitemLoop);

      for(std::size_t i=0; i<pSPS -> num_long_term_ref_pics_sps; i++)
      {
        pspsItem -> addChild(new QTreeWidgetItem(QStringList("lt_ref_pic_poc_lsb_sps[ " + QString::number(i) + "] = " + QString::number(pSPS -> lt_ref_pic_poc_lsb_sps[i]))));
        pspsItem -> addChild(new QTreeWidgetItem(QStringList("used_by_curr_pic_lt_sps_flag[ " + QString::number(i) + "] = " + QString::number(pSPS -> used_by_curr_pic_lt_sps_flag[i]))));
      }
    }
  }

  pspsItem -> addChild(new QTreeWidgetItem(QStringList("sps_temporal_mvp_enabled_flag = " + QString::number(pSPS -> sps_temporal_mvp_enabled_flag))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("strong_intra_smoothing_enabled_flag = " + QString::number(pSPS -> strong_intra_smoothing_enabled_flag))));
  pspsItem -> addChild(new QTreeWidgetItem(QStringList("vui_parameters_present_flag = " + QString::number(pSPS -> vui_parameters_present_flag))));

  if(pSPS -> vui_parameters_present_flag)
  {
    pitem = new QTreeWidgetItem(QStringList("if( vui_parameters_present_flag )"));
    pspsItem -> addChild(pitem);
    QTreeWidgetItem *pVuiItem = new QTreeWidgetItem(QStringList("vui_parameters"));
    pitem -> addChild(pVuiItem);

    createVuiParameters(pSPS -> vui_parameters, pSPS -> sps_max_sub_layers_minus1, pVuiItem);
  }

  pspsItem -> addChild(new QTreeWidgetItem(QStringList("sps_extension_flag = " + QString::number(pSPS -> sps_extension_flag))));
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
  {
    pitem = new QTreeWidgetItem(QStringList("if(cu_qp_delta_enabled_flag)"));
    pppsItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("diff_cu_qp_delta_depth = " + QString::number(pPPS -> diff_cu_qp_delta_depth))));
  }

  pppsItem -> addChild(new QTreeWidgetItem(QStringList("pps_cb_qp_offset = " + QString::number(pPPS -> pps_cb_qp_offset))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("pps_cr_qp_offset = " + QString::number(pPPS -> pps_cr_qp_offset))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("pps_slice_chroma_qp_offsets_present_flag = " + QString::number(pPPS -> pps_slice_chroma_qp_offsets_present_flag))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("weighted_pred_flag = " + QString::number(pPPS -> weighted_pred_flag))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("weighted_bipred_flag = " + QString::number(pPPS -> weighted_bipred_flag))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("transquant_bypass_enabled_flag = " + QString::number(pPPS -> transquant_bypass_enabled_flag))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("tiles_enabled_flag = " + QString::number(pPPS -> tiles_enabled_flag))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("entropy_coding_sync_enabled_flag = " + QString::number(pPPS -> entropy_coding_sync_enabled_flag))));


  if(pPPS -> tiles_enabled_flag)
  {
    pitem = new QTreeWidgetItem(QStringList("if(tiles_enabled_flag)"));
    pppsItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("num_tile_columns_minus1 = " + QString::number(pPPS -> num_tile_columns_minus1))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("num_tile_rows_minus1 = " + QString::number(pPPS -> num_tile_rows_minus1))));

    pitem -> addChild(new QTreeWidgetItem(QStringList("uniform_spacing_flag = " + QString::number(pPPS -> uniform_spacing_flag))));

    if(!pPPS -> uniform_spacing_flag)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if(!uniform_spacing_flag)"));
      pitem -> addChild(pitemSecond);

      QString str = "column_width_minus1 = {\n\t";
      for(int i=0; i<(int)pPPS -> num_tile_columns_minus1 - 1; i++)
      {
        str += QString::number(pPPS -> column_width_minus1[i]) + ", ";
        if((i+1) % 8 == 0)
          str += "\n\t";
      }
      if(pPPS -> num_tile_columns_minus1 > 0)
        str += QString::number(pPPS -> column_width_minus1[pPPS -> num_tile_columns_minus1 - 1]) + " \n}";
      else 
        str += "}";

      pitemSecond -> addChild(new QTreeWidgetItem(QStringList(str)));

      str = "row_height_minus1 = {\n\t";
      for(int i=0; i<(int)pPPS -> num_tile_rows_minus1 - 1; i++)
      {
        str += QString::number(pPPS -> row_height_minus1[i]) + ", ";
        if((i+1) % 8 == 0)
          str += "\n\t";
      }
      if(pPPS -> num_tile_rows_minus1 > 0)
        str += QString::number(pPPS -> row_height_minus1[pPPS -> num_tile_rows_minus1 - 1]) + " \n}";
      else 
        str += "}";

      pitemSecond -> addChild(new QTreeWidgetItem(QStringList(str)));
    }

    pitem -> addChild(new QTreeWidgetItem(QStringList("loop_filter_across_tiles_enabled_flag = " + QString::number(pPPS -> loop_filter_across_tiles_enabled_flag))));
  }

  pppsItem -> addChild(new QTreeWidgetItem(QStringList("pps_loop_filter_across_slices_enabled_flag = " + QString::number(pPPS -> pps_loop_filter_across_slices_enabled_flag))));

  pppsItem -> addChild(new QTreeWidgetItem(QStringList("deblocking_filter_control_present_flag = " + QString::number(pPPS -> deblocking_filter_control_present_flag))));

  if(pPPS -> deblocking_filter_control_present_flag)
  {

    pitem = new QTreeWidgetItem(QStringList("if(deblocking_filter_control_present_flag)"));
    pppsItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("deblocking_filter_override_enabled_flag = " + QString::number(pPPS -> deblocking_filter_override_enabled_flag))));

    pitem -> addChild(new QTreeWidgetItem(QStringList("pps_deblocking_filter_disabled_flag = " + QString::number(pPPS -> pps_deblocking_filter_disabled_flag))));

    if(!pPPS -> pps_deblocking_filter_disabled_flag)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if(!pps_deblocking_filter_disabled_flag)"));
      pitem -> addChild(pitemSecond);

      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("pps_beta_offset_div2 = " + QString::number(pPPS -> pps_beta_offset_div2))));
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("pps_tc_offset_div2 = " + QString::number(pPPS -> pps_tc_offset_div2))));
    }
  }


  pppsItem -> addChild(new QTreeWidgetItem(QStringList("pps_scaling_list_data_present_flag = " + QString::number(pPPS -> pps_scaling_list_data_present_flag))));

  if(pPPS -> pps_scaling_list_data_present_flag)
  {
    pitem = new QTreeWidgetItem(QStringList("if(pps_scaling_list_data_present_flag)"));
    pppsItem -> addChild(pitem);

    QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("scaling_list_data"));
    pitem -> addChild(pitemSecond);
    createScalingListData(pPPS -> scaling_list_data, pitemSecond);

  }

  pppsItem -> addChild(new QTreeWidgetItem(QStringList("lists_modification_present_flag = " + QString::number(pPPS -> lists_modification_present_flag))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("log2_parallel_merge_level_minus2 = " + QString::number(pPPS -> log2_parallel_merge_level_minus2))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("slice_segment_header_extension_present_flag = " + QString::number(pPPS -> slice_segment_header_extension_present_flag))));
  pppsItem -> addChild(new QTreeWidgetItem(QStringList("pps_extension_flag = " + QString::number(pPPS -> pps_extension_flag))));
}

void SyntaxViewer::createSlice(std::shared_ptr<HEVC::Slice> pSlice)
{
  if(pSlice -> m_processFailed)
    return;

  std::shared_ptr<HEVC::PPS> pPPS = m_ppsMap[pSlice -> slice_pic_parameter_set_id];
  if(!pPPS)
    return;

  int32_t spsId = pPPS -> pps_seq_parameter_set_id;
  QTreeWidgetItem *pitemDepend, *pitemThird;
  QTreeWidgetItem *psliceItem = new QTreeWidgetItem(QStringList("Slice"));
  addTopLevelItem(psliceItem);

  QTreeWidgetItem *pitem;
  psliceItem -> addChild(new QTreeWidgetItem(QStringList("first_slice_segment_in_pic_flag = " + QString::number(pSlice -> first_slice_segment_in_pic_flag))));

  if(pSlice -> m_nalUnitType >= HEVC::NAL_BLA_W_LP && pSlice -> m_nalUnitType <= HEVC::NAL_IRAP_VCL23)
  {
    pitem = new QTreeWidgetItem(QStringList("if ( nal_unit_type >= BLA_W_LP && nal_unit_type <= RSV_IRAP_VCL23)"));
    psliceItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("no_output_of_prior_pics_flag = " + QString::number(pSlice -> no_output_of_prior_pics_flag))));
  }

  psliceItem -> addChild(new QTreeWidgetItem(QStringList("slice_pic_parameter_set_id = " + QString::number(pSlice -> slice_pic_parameter_set_id))));

  if(!pSlice -> first_slice_segment_in_pic_flag)
  {
    pitem = new QTreeWidgetItem(QStringList("if (!first_slice_segment_in_pic_flag)"));
    psliceItem -> addChild(pitem);

    if(pPPS -> dependent_slice_segments_enabled_flag)
    {
      pitemDepend = new QTreeWidgetItem(QStringList("pps -> dependent_slice_segments_enabled_flag = " + QString::number(pPPS -> dependent_slice_segments_enabled_flag)));
      pitem -> addChild(pitemDepend);

      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("dependent_slice_segment_flag = " + QString::number(pSlice -> dependent_slice_segment_flag))));
    }
    pitem -> addChild(new QTreeWidgetItem(QStringList("slice_segment_address = " + QString::number(pSlice -> slice_segment_address))));
  }

  if(!pSlice -> dependent_slice_segment_flag)
  {
    pitem = new QTreeWidgetItem(QStringList("if (!dependent_slice_segment_flag)"));
    psliceItem -> addChild(pitem);

    long num_extra_slice_header_bits = pPPS -> num_extra_slice_header_bits;

    QString str;
    if(num_extra_slice_header_bits > 0)
    {
      str = "slice_reserved_undetermined_flag = {\n\t";

      if(num_extra_slice_header_bits > pSlice -> slice_reserved_undetermined_flag.size())
        return;

      for(long i=0; i<num_extra_slice_header_bits - 1; i++)
        str += QString::number(pSlice -> slice_reserved_undetermined_flag[i]) + ", ";

      str += QString::number(pSlice -> slice_reserved_undetermined_flag[num_extra_slice_header_bits - 1]) + ", ";
    }
    else
      str = "slice_reserved_undetermined_flag = { }";

    pitem -> addChild(new QTreeWidgetItem(QStringList(str)));
    pitem -> addChild(new QTreeWidgetItem(QStringList("slice_type = " + QString::number(pSlice -> slice_type))));
    
    if(pPPS -> output_flag_present_flag)
    {
      pitemDepend = new QTreeWidgetItem(QStringList("if (output_flag_present_flag)"));
      pitem -> addChild(pitemDepend);
      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("pic_output_flag = " + QString::number(pSlice -> pic_output_flag))));
    }


    if(!m_spsMap[spsId])
      return;

    if(m_spsMap[spsId] -> separate_colour_plane_flag)
    {
      pitemDepend = new QTreeWidgetItem(QStringList("if (sps -> separate_colour_plane_flag)"));
      pitem -> addChild(pitemDepend);
      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("colour_plane_id = " + QString::number(pSlice -> colour_plane_id))));
    }    
    bool IdrPicFlag = pSlice -> m_nalUnitType == HEVC::NAL_IDR_W_RADL || pSlice -> m_nalUnitType == HEVC::NAL_IDR_N_LP;

    if(!IdrPicFlag)
    {
      pitemDepend = new QTreeWidgetItem(QStringList("if (nal_unit_type != IDR_W_RADL && nal_unit_type != IDR_N_LP)"));
      pitem -> addChild(pitemDepend);

      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("pic_order_cnt_lsb = " + QString::number(pSlice -> pic_order_cnt_lsb))));

      QTreeWidgetItem *pitemThird = new QTreeWidgetItem(QStringList("short_term_ref_pic_set_sps_flag = " + QString::number(pSlice -> short_term_ref_pic_set_sps_flag)));
      pitemDepend -> addChild(pitemThird);
      if(!pSlice -> short_term_ref_pic_set_sps_flag)
      {
        pitemThird = new QTreeWidgetItem(QStringList("if (!short_term_ref_pic_set_sps_flag)"));
        pitemDepend -> addChild(pitemThird);
        QTreeWidgetItem *pStrpc = new QTreeWidgetItem(QStringList("short_term_ref_pic_set(" + QString::number(m_spsMap[spsId] -> num_short_term_ref_pic_sets) + ")"));
        pitemThird -> addChild(pStrpc);
        createShortTermRefPicSet(m_spsMap[spsId] -> num_short_term_ref_pic_sets, pSlice -> short_term_ref_pic_set, m_spsMap[spsId] -> num_short_term_ref_pic_sets, m_spsMap[spsId] -> short_term_ref_pic_set, pStrpc);
      }
      else if(m_spsMap[spsId] -> num_short_term_ref_pic_sets > 1)
      {
        pitemThird = new QTreeWidgetItem(QStringList("if (short_term_ref_pic_set_sps_flag && num_long_term_ref_pics_sps > 0)"));
        pitemDepend -> addChild(pitemThird);

        pitemThird -> addChild(new QTreeWidgetItem(QStringList("short_term_ref_pic_set_idx = " + QString::number(pSlice -> short_term_ref_pic_set_idx))));
      }

      if(m_spsMap[spsId] -> long_term_ref_pics_present_flag)
      {
        pitemThird = new QTreeWidgetItem(QStringList("if (long_term_ref_pics_present_flag)"));
        pitemDepend -> addChild(pitemThird);

        if(m_spsMap[spsId] -> num_long_term_ref_pics_sps > 0)
        {
          QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if (num_long_term_ref_pics_sps)"));
          pitemThird -> addChild(pitemSecond);

          pitemSecond -> addChild(new QTreeWidgetItem(QStringList("num_long_term_sps = " + QString::number(pSlice -> num_long_term_sps))));
        }

        pitemThird -> addChild(new QTreeWidgetItem(QStringList("num_long_term_pics = " + QString::number(pSlice -> num_long_term_pics))));

        std::size_t num_long_term = pSlice -> num_long_term_sps + pSlice -> num_long_term_pics;
        QTreeWidgetItem *pitemLoop = new QTreeWidgetItem(QStringList("for( i = 0; i < num_long_term_sps + num_long_term_pics; i++ )"));
        pitemThird -> addChild(pitemLoop);

        for(std::size_t i=0; i < num_long_term; i++)
        {
          if(i < pSlice -> num_long_term_sps) 
          {
            QTreeWidgetItem *pitem1 = new QTreeWidgetItem(QStringList("if (i < num_long_term_sps)"));
            pitemLoop -> addChild(pitem1);

            if(m_spsMap[spsId] -> num_long_term_ref_pics_sps > 1)
            {
              QTreeWidgetItem *pitem2 = new QTreeWidgetItem(QStringList("if (num_long_term_ref_pics_sps > 1)"));
              pitem1 -> addChild(pitem2);
              pitem2 -> addChild(new QTreeWidgetItem(QStringList("lt_idx_sps[" + QString::number(i) + "] = " + QString::number(pSlice -> lt_idx_sps[i]))));
            }
          }
          else
          {
            QTreeWidgetItem *pitem1 = new QTreeWidgetItem(QStringList("if (i >= num_long_term_sps)"));
            pitemLoop -> addChild(pitem1);
            pitem1 -> addChild(new QTreeWidgetItem(QStringList("poc_lsb_lt[" + QString::number(i) + "] = " + QString::number(pSlice -> poc_lsb_lt[i]))));
            pitem1 -> addChild(new QTreeWidgetItem(QStringList("used_by_curr_pic_lt_flag[" + QString::number(i) + "] = " + QString::number(pSlice -> used_by_curr_pic_lt_flag[i]))));
          }

          pitemLoop -> addChild(new QTreeWidgetItem(QStringList("delta_poc_msb_present_flag[" + QString::number(i) + "] = " + QString::number(pSlice -> delta_poc_msb_present_flag[i]))));
          if(pSlice -> delta_poc_msb_present_flag[i])
          {
            QTreeWidgetItem *pitem1 = new QTreeWidgetItem(QStringList("if (delta_poc_msb_present_flag[" + QString::number(i) + "])"));
            pitemLoop -> addChild(pitem1);
            pitem1 -> addChild(new QTreeWidgetItem(QStringList("delta_poc_msb_cycle_lt[" + QString::number(i) + "] = " + QString::number(pSlice -> delta_poc_msb_cycle_lt[i]))));
          }
        }
      }

      if(m_spsMap[spsId] -> sps_temporal_mvp_enabled_flag)
      {
        QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if (sps_temporal_mvp_enabled_flag)"));
        pitemDepend -> addChild(pitemSecond);
        pitemSecond -> addChild(new QTreeWidgetItem(QStringList("slice_temporal_mvp_enabled_flag = " + QString::number(pSlice -> slice_temporal_mvp_enabled_flag))));
      }
    }

    if(m_spsMap[spsId] -> sample_adaptive_offset_enabled_flag)
    {
      pitemDepend = new QTreeWidgetItem(QStringList("if (sample_adaptive_offset_enabled_flag)"));
      pitem -> addChild(pitemDepend);

      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("slice_sao_luma_flag = " + QString::number(pSlice -> slice_sao_luma_flag))));
      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("slice_sao_chroma_flag = " + QString::number(pSlice -> slice_sao_chroma_flag))));
    }

    if(pSlice -> slice_type == SLICE_B || pSlice -> slice_type == SLICE_P)
    {
      pitemDepend = new QTreeWidgetItem(QStringList("if (slice_type == P || slice_type == B)"));
      pitem -> addChild(pitemDepend);

      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("num_ref_idx_active_override_flag = " + QString::number(pSlice -> num_ref_idx_active_override_flag))));

      if(pSlice -> num_ref_idx_active_override_flag)
      {
        QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if (num_ref_idx_active_override_flag)"));
        pitemDepend -> addChild(pitemSecond);

        pitemSecond -> addChild(new QTreeWidgetItem(QStringList("num_ref_idx_l0_active_minus1 = " + QString::number(pSlice -> num_ref_idx_l0_active_minus1))));

        if(pSlice -> slice_type == SLICE_B)
        {
          QTreeWidgetItem *pitemThird = new QTreeWidgetItem(QStringList("if (slice_type == B)"));
          pitemSecond -> addChild(pitemThird);

          pitemThird -> addChild(new QTreeWidgetItem(QStringList("num_ref_idx_l1_active_minus1 = " + QString::number(pSlice -> num_ref_idx_l1_active_minus1))));
        }
      }

      if(pPPS -> lists_modification_present_flag)
      {
        std::size_t NumPocTotalCurr = calcNumPocTotalCurr(pSlice, m_spsMap[spsId]);

        if(NumPocTotalCurr > 1)
        {
          pitemThird = new QTreeWidgetItem(QStringList("if (lists_modification_present_flag && NumPocTotalCurr > 1 )"));
          pitemDepend -> addChild(pitemThird);
          QTreeWidgetItem *pitemRplMod = new QTreeWidgetItem(QStringList("short_term_ref_pic_set(" + QString::number(m_spsMap[spsId] -> num_short_term_ref_pic_sets) + ")"));
          pitemThird -> addChild(pitemRplMod);


          createRefPicListModification(pSlice -> ref_pic_lists_modification, pitemRplMod);
        }        
      }

      if(pSlice -> slice_type == SLICE_B)
      {
        pitemThird = new QTreeWidgetItem(QStringList("if (slice_type == B)"));
        pitemDepend -> addChild(pitemThird);
        pitemThird -> addChild(new QTreeWidgetItem(QStringList("mvd_l1_zero_flag = " + QString::number(pSlice -> mvd_l1_zero_flag))));
      }

      if(pPPS -> cabac_init_present_flag)
      {
        pitemThird = new QTreeWidgetItem(QStringList("if (cabac_init_present_flag)"));
        pitemDepend -> addChild(pitemThird);
        pitemThird-> addChild(new QTreeWidgetItem(QStringList("cabac_init_flag = " + QString::number(pSlice -> cabac_init_flag))));
      }

      if(pSlice -> slice_temporal_mvp_enabled_flag)
      {
        pitemThird = new QTreeWidgetItem(QStringList("if (slice_temporal_mvp_enabled_flag)"));
        pitemDepend -> addChild(pitemThird);

        if(pSlice -> slice_type == SLICE_B)
        {
          QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if (slice_type == B)"));
          pitemThird -> addChild(pitemSecond);
          pitemSecond -> addChild(new QTreeWidgetItem(QStringList("collocated_from_l0_flag = " + QString::number(pSlice -> collocated_from_l0_flag))));
        }

        if(pSlice -> collocated_from_l0_flag && pSlice -> num_ref_idx_l0_active_minus1 ||
            !pSlice -> collocated_from_l0_flag && pSlice -> num_ref_idx_l1_active_minus1)
        {
          QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if ((collocated_from_l0_flag && num_ref_idx_l0_active_minus1 > 0 ) || (!collocated_from_l0_flag && num_ref_idx_l1_active_minus1 > 0))"));
          pitemThird -> addChild(pitemSecond);

          pitemSecond -> addChild(new QTreeWidgetItem(QStringList("collocated_ref_idx = " + QString::number(pSlice -> collocated_ref_idx))));
        }
      }

      if(pPPS -> weighted_pred_flag && pSlice -> slice_type == SLICE_P ||
        pPPS -> weighted_bipred_flag && pSlice -> slice_type == SLICE_B)
      {
        pitemThird = new QTreeWidgetItem(QStringList("if ((weighted_pred_flag && slice_type == P) || (weighted_bipred_flag && slice_type == B))"));
        pitemDepend -> addChild(pitemThird);

        QTreeWidgetItem *pitempwt = new QTreeWidgetItem(QStringList("pred_weight_table"));
        pitemThird -> addChild(pitempwt);

        createPredWeightTable(pSlice -> pred_weight_table, pSlice, pitempwt);
      }

      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("five_minus_max_num_merge_cand = " + QString::number(pSlice -> five_minus_max_num_merge_cand))));
    }

    pitem -> addChild(new QTreeWidgetItem(QStringList("slice_qp_delta = " + QString::number(pSlice -> slice_qp_delta))));


    if(pPPS -> pps_slice_chroma_qp_offsets_present_flag)
    {
      pitemDepend = new QTreeWidgetItem(QStringList("if (pps_slice_chroma_qp_offsets_present_flag)"));
      pitem -> addChild(pitemDepend);

      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("slice_cb_qp_offset = " + QString::number(pSlice -> slice_cb_qp_offset))));
      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("slice_cr_qp_offset = " + QString::number(pSlice -> slice_cr_qp_offset))));
    }

    if(pPPS -> deblocking_filter_override_enabled_flag)
    {
      pitemDepend = new QTreeWidgetItem(QStringList("if (deblocking_filter_override_enabled_flag)"));
      pitem -> addChild(pitemDepend);
      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("deblocking_filter_override_flag = " + QString::number(pSlice -> deblocking_filter_override_flag))));
    }

    if(pSlice -> deblocking_filter_override_flag)
    {
      pitemDepend = new QTreeWidgetItem(QStringList("if (deblocking_filter_override_flag)"));
      pitem -> addChild(pitemDepend);

      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("slice_deblocking_filter_disabled_flag = " + QString::number(pSlice -> slice_deblocking_filter_disabled_flag))));

      if(!pSlice -> slice_deblocking_filter_disabled_flag)
      {
        pitemThird = new QTreeWidgetItem(QStringList("if (!slice_deblocking_filter_disabled_flag)"));
        pitemDepend -> addChild(pitemThird);

        pitemThird -> addChild(new QTreeWidgetItem(QStringList("slice_beta_offset_div2 = " + QString::number(pSlice -> slice_beta_offset_div2))));
        pitemThird -> addChild(new QTreeWidgetItem(QStringList("slice_tc_offset_div2 = " + QString::number(pSlice -> slice_tc_offset_div2))));
      }
    }

    if(pPPS -> pps_loop_filter_across_slices_enabled_flag && 
      (pSlice -> slice_sao_luma_flag || pSlice -> slice_sao_chroma_flag || !pSlice -> slice_deblocking_filter_disabled_flag))
    {
      pitemDepend = new QTreeWidgetItem(QStringList("if (pps_loop_filter_across_slices_enabled_flag && (slice_sao_luma_flag || slice_sao_chroma_flag || !slice_deblocking_filter_disabled_flag ))"));
      pitem -> addChild(pitemDepend);

      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("slice_loop_filter_across_slices_enabled_flag = " + QString::number(pSlice -> slice_loop_filter_across_slices_enabled_flag))));
    }
  }

  if(pPPS -> tiles_enabled_flag || pPPS -> entropy_coding_sync_enabled_flag)
  {
    pitem = new QTreeWidgetItem(QStringList("if (tiles_enabled_flag || entropy_coding_sync_enabled_flag)"));
    psliceItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("num_entry_point_offsets = " + QString::number(pSlice -> num_entry_point_offsets))));

    if(pSlice -> num_entry_point_offsets > 0)
    {
      pitemThird = new QTreeWidgetItem(QStringList("if (num_entry_point_offsets > 0)"));
      pitem -> addChild(pitemThird);

      pitemThird -> addChild(new QTreeWidgetItem(QStringList("offset_len_minus1 = " + QString::number(pSlice -> offset_len_minus1))));

      QTreeWidgetItem *pitemLoop = new QTreeWidgetItem(QStringList("for( i = 0; i < num_entry_point_offsets; i++ )"));
      pitemThird -> addChild(pitemLoop);

      for(std::size_t i=0; i<pSlice -> num_entry_point_offsets; i++)
        pitemLoop -> addChild(new QTreeWidgetItem(QStringList("entry_point_offset_minus1[" + QString::number(i) + "] = " + QString::number(pSlice -> entry_point_offset_minus1[i]))));
    }
  }

  if(pPPS -> slice_segment_header_extension_present_flag)
  {
    pitem = new QTreeWidgetItem(QStringList("if (slice_segment_header_extension_present_flag)"));
    psliceItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("slice_segment_header_extension_length = " + QString::number(pSlice -> slice_segment_header_extension_length))));

    QTreeWidgetItem *pitemLoop = new QTreeWidgetItem(QStringList("for( i = 0; i < slice_segment_header_extension_length; i++ )"));
    pitem -> addChild(pitemLoop);

    for(std::size_t i=0; i<pSlice -> slice_segment_header_extension_length; i++)
    {
      pitemLoop -> addChild(new QTreeWidgetItem(QStringList("slice_segment_header_extension_data_byte[" + QString::number(i) + "] = " + QString::number(pSlice -> slice_segment_header_extension_data_byte[i]))));
    }
  }
}

void SyntaxViewer::createAUD(std::shared_ptr<HEVC::AUD> pAUD)
{
  QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("AUD"));
  addTopLevelItem(pitem);
  pitem -> addChild(new QTreeWidgetItem(QStringList("pic_type = " + QString::number(pAUD -> pic_type))));
}


void SyntaxViewer::createSEI(std::shared_ptr<HEVC::SEI> pSEI)
{
  QTreeWidgetItem *pitemSEI = new QTreeWidgetItem(QStringList("SEI"));
  addTopLevelItem(pitemSEI);



  for(std::size_t i=0; i< pSEI -> sei_message.size(); i++)
  {
    std::size_t payloadType = 0;
    std::size_t payloadSize = 0;

    QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("sei_message(" + QString::number(i) + ")"));
    pitemSEI -> addChild(pitem);
    
    if(pSEI -> sei_message[i].num_payload_type_ff_bytes)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("while( next_bits( 8 ) == 0xFF )"));
      pitem -> addChild(pitemSecond);
      for(std::size_t i=0; i<pSEI -> sei_message[i].num_payload_type_ff_bytes; i++)
      {
        pitemSecond -> addChild(new QTreeWidgetItem(QStringList("0xFF")));
        payloadType += 255;
      }
    }

    pitem -> addChild(new QTreeWidgetItem(QStringList("last_payload_type_byte = " + QString::number(pSEI -> sei_message[i].last_payload_type_byte))));
    payloadType += pSEI -> sei_message[i].last_payload_type_byte;


    if(pSEI -> sei_message[i].num_payload_size_ff_bytes)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("while( next_bits( 8 ) == 0xFF )"));
      pitem -> addChild(pitemSecond);
      for(std::size_t i=0; i<pSEI -> sei_message[i].num_payload_size_ff_bytes; i++)
      {
        pitemSecond -> addChild(new QTreeWidgetItem(QStringList("0xFF")));
        payloadSize += 255;
      }
    }

    pitem -> addChild(new QTreeWidgetItem(QStringList("last_payload_type_byte = " + QString::number(pSEI -> sei_message[i].last_payload_size_byte))));

    payloadSize += pSEI -> sei_message[i].last_payload_size_byte;
    pitem -> addChild(new QTreeWidgetItem(QStringList("sei_payload(" + QString::number(payloadType) + ", " + QString::number(payloadSize) + ")")));
  }
}


void SyntaxViewer::createProfileTierLevel(const HEVC::ProfileTierLevel &ptl, QTreeWidgetItem *pPtlItem)
{

  pPtlItem -> addChild(new QTreeWidgetItem(QStringList("general_profile_space = " + QString::number(ptl.general_profile_space))));
  pPtlItem -> addChild(new QTreeWidgetItem(QStringList("general_tier_flag = " + QString::number(ptl.general_tier_flag))));
  pPtlItem -> addChild(new QTreeWidgetItem(QStringList("general_profile_idc = " + QString::number(ptl.general_profile_idc))));

  QString str = "general_profile_compatibility_flag[i] = { \n\t";
  for(std::size_t i=0; i<31; i++)
  {
    str += QString::number(ptl.general_profile_compatibility_flag[i]) + ", ";
    if((i+1) % 8 == 0)
      str += "\n\t";
  }
  str += QString::number(ptl.general_profile_compatibility_flag[31]) + "\n}";
  pPtlItem -> addChild(new QTreeWidgetItem(QStringList(str)));

  pPtlItem -> addChild(new QTreeWidgetItem(QStringList("general_progressive_source_flag = " + QString::number(ptl.general_progressive_source_flag))));
  pPtlItem -> addChild(new QTreeWidgetItem(QStringList("general_interlaced_source_flag = " + QString::number(ptl.general_interlaced_source_flag))));
  pPtlItem -> addChild(new QTreeWidgetItem(QStringList("general_non_packed_constraint_flag = " + QString::number(ptl.general_non_packed_constraint_flag))));
  pPtlItem -> addChild(new QTreeWidgetItem(QStringList("general_frame_only_constraint_flag = " + QString::number(ptl.general_frame_only_constraint_flag))));
  pPtlItem -> addChild(new QTreeWidgetItem(QStringList("general_level_idc = " + QString::number(ptl.general_level_idc))));

  if(ptl.sub_layer_profile_present_flag.size() == 0)
  {
    pPtlItem -> addChild(new QTreeWidgetItem(QStringList("sub_layer_profile_present_flag = {}")));
    pPtlItem -> addChild(new QTreeWidgetItem(QStringList("sub_layer_level_present_flag = {}")));
  }
  else
  {
    str = "sub_layer_profile_present_flag = {\n\t";
    for(std::size_t i=0; i<ptl.sub_layer_profile_present_flag.size() - 1; i++)
    {
      str += QString::number(ptl.sub_layer_profile_present_flag[i]) + ", ";
      if((i+1) % 8 == 0)
        str += "\n\t";
    }
    str += QString::number(ptl.sub_layer_profile_present_flag[ptl.sub_layer_profile_present_flag.size() - 1]) + " \n}";
    pPtlItem -> addChild(new QTreeWidgetItem(QStringList(str)));

    str = "sub_layer_level_present_flag = {\n\t";
    for(std::size_t i=0; i<ptl.sub_layer_profile_present_flag.size() - 1; i++)
    {
      str += QString::number(ptl.sub_layer_level_present_flag[i]) + ", ";
      if((i+1) % 8 == 0)
        str += "\n\t";
    }
    str += QString::number(ptl.sub_layer_level_present_flag[ptl.sub_layer_profile_present_flag.size() - 1]) + " \n}";
    pPtlItem -> addChild(new QTreeWidgetItem(QStringList(str)));
  }

  std::size_t maxNumSubLayersMinus1 = ptl.sub_layer_profile_present_flag.size();
  bool needLoop = false;

  for(std::size_t i=0; i<maxNumSubLayersMinus1 && !needLoop; i++)
  {
    if(ptl.sub_layer_profile_present_flag[i])
      needLoop = true;

    if(ptl.sub_layer_level_present_flag[i])
      needLoop = true;
  }      

  if(needLoop)
  {
      QTreeWidgetItem *pitemLoop = new QTreeWidgetItem(QStringList("for( i = 0; i < maxNumSubLayersMinus1; i++ )"));
      pPtlItem -> addChild(pitemLoop);

      for(std::size_t i=0; i<maxNumSubLayersMinus1; i++)
      {
        if(ptl.sub_layer_profile_present_flag[i])
        {
          QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if( sub_layer_profile_present_flag[" + QString::number(i) +  "] )"));
          pitemLoop -> addChild(pitem);


          pitem -> addChild(new QTreeWidgetItem(QStringList("sub_layer_profile_space[" + QString::number(i) + "] = " + QString::number(ptl.sub_layer_profile_space[i]))));
          pitem -> addChild(new QTreeWidgetItem(QStringList("sub_layer_tier_flag[" + QString::number(i) + "] = " + QString::number(ptl.sub_layer_tier_flag[i]))));
          pitem -> addChild(new QTreeWidgetItem(QStringList("sub_layer_profile_idc[" + QString::number(i) + "] = " + QString::number(ptl.sub_layer_profile_idc[i]))));

          QString str = "sub_layer_profile_compatibility_flag = { \n\t";
          for(std::size_t j=0; j<31; j++)
          {
            str += QString::number(ptl.sub_layer_profile_compatibility_flag[i][j]) + ", ";
            if((j+1) % 8 == 0)
              str += "\n\t";
          }
          str +=  QString::number(ptl.sub_layer_profile_compatibility_flag[i][31]) + " \n}";
          pitem -> addChild(new QTreeWidgetItem(QStringList(str)));


          pitem -> addChild(new QTreeWidgetItem(QStringList("sub_layer_progressive_source_flag[" + QString::number(i) + "] = " + QString::number(ptl.sub_layer_progressive_source_flag[i]))));
          pitem -> addChild(new QTreeWidgetItem(QStringList("sub_layer_interlaced_source_flag[" + QString::number(i) + "] = " + QString::number(ptl.sub_layer_interlaced_source_flag[i]))));
          pitem -> addChild(new QTreeWidgetItem(QStringList("sub_layer_non_packed_constraint_flag[" + QString::number(i) + "] = " + QString::number(ptl.sub_layer_non_packed_constraint_flag[i]))));
          pitem -> addChild(new QTreeWidgetItem(QStringList("sub_layer_frame_only_constraint_flag[" + QString::number(i) + "] = " + QString::number(ptl.sub_layer_frame_only_constraint_flag[i]))));
        }

        if(ptl.sub_layer_level_present_flag[i])
        {
          QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if( sub_layer_level_present_flag[" + QString::number(i) +  "] )"));
          pitemLoop -> addChild(pitem);
          pitem -> addChild(new QTreeWidgetItem(QStringList("sub_layer_level_idc[" + QString::number(i) + "] = " + QString::number(ptl.sub_layer_level_idc[i]))));
        }
      }
  }
}


void SyntaxViewer::createVuiParameters(const HEVC::VuiParameters &vui, std::size_t maxNumSubLayersMinus1, QTreeWidgetItem *pVuiItem)
{
  pVuiItem -> addChild(new QTreeWidgetItem(QStringList("aspect_ratio_info_present_flag = " + QString::number(vui.aspect_ratio_info_present_flag))));

  if(vui.aspect_ratio_info_present_flag)
  {
    QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if( aspect_ratio_info_present_flag )"));
    pVuiItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("aspect_ratio_idc = " + QString::number(vui.aspect_ratio_idc))));

    if(vui.aspect_ratio_idc == 255) //EXTENDED_SAR
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( aspect_ratio_idc == EXTENDED_SAR )"));
      pitem -> addChild(pitemSecond);

      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("sar_width = " + QString::number(vui.sar_width))));
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("sar_height = " + QString::number(vui.sar_height))));
    }
  }

  pVuiItem -> addChild(new QTreeWidgetItem(QStringList("overscan_info_present_flag = " + QString::number(vui.overscan_info_present_flag))));

  if(vui.overscan_info_present_flag)
  {
    QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if( overscan_info_present_flag )"));
    pVuiItem -> addChild(pitem);
    pitem -> addChild(new QTreeWidgetItem(QStringList("overscan_appropriate_flag = " + QString::number(vui.overscan_appropriate_flag))));
  }

  pVuiItem -> addChild(new QTreeWidgetItem(QStringList("video_signal_type_present_flag = " + QString::number(vui.video_signal_type_present_flag))));

  if(vui.video_signal_type_present_flag)
  {
    QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if( video_signal_type_present_flag )"));
    pVuiItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("video_format = " + QString::number(vui.video_format))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("video_full_range_flag = " + QString::number(vui.video_full_range_flag))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("colour_description_present_flag = " + QString::number(vui.colour_description_present_flag))));

    if(vui.colour_description_present_flag)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( colour_description_present_flag )"));
      pitem -> addChild(pitemSecond);

      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("colour_primaries = " + QString::number(vui.colour_primaries))));
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("transfer_characteristics = " + QString::number(vui.transfer_characteristics))));
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("matrix_coeffs = " + QString::number(vui.matrix_coeffs))));
    }
  }

  pVuiItem -> addChild(new QTreeWidgetItem(QStringList("chroma_loc_info_present_flag = " + QString::number(vui.chroma_loc_info_present_flag))));

  if(vui.chroma_loc_info_present_flag)
  {
    QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if( chroma_loc_info_present_flag )"));
    pVuiItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("chroma_sample_loc_type_top_field = " + QString::number(vui.chroma_sample_loc_type_top_field))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("chroma_sample_loc_type_bottom_field = " + QString::number(vui.chroma_sample_loc_type_bottom_field))));
  }

  pVuiItem -> addChild(new QTreeWidgetItem(QStringList("neutral_chroma_indication_flag = " + QString::number(vui.neutral_chroma_indication_flag))));
  pVuiItem -> addChild(new QTreeWidgetItem(QStringList("field_seq_flag = " + QString::number(vui.field_seq_flag))));
  pVuiItem -> addChild(new QTreeWidgetItem(QStringList("frame_field_info_present_flag = " + QString::number(vui.frame_field_info_present_flag))));
  pVuiItem -> addChild(new QTreeWidgetItem(QStringList("default_display_window_flag = " + QString::number(vui.default_display_window_flag))));


  if(vui.default_display_window_flag)
  {
    QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if( default_display_window_flag )"));
    pVuiItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("def_disp_win_left_offset = " + QString::number(vui.def_disp_win_left_offset))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("def_disp_win_right_offset = " + QString::number(vui.def_disp_win_right_offset))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("def_disp_win_top_offset = " + QString::number(vui.def_disp_win_top_offset))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("def_disp_win_bottom_offset = " + QString::number(vui.def_disp_win_bottom_offset))));
  }

  pVuiItem -> addChild(new QTreeWidgetItem(QStringList("vui_timing_info_present_flag = " + QString::number(vui.vui_timing_info_present_flag))));

  if(vui.vui_timing_info_present_flag)
  {
    QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if( vui_timing_info_present_flag )"));
    pVuiItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("vui_num_units_in_tick = " + QString::number(vui.vui_num_units_in_tick))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("vui_time_scale = " + QString::number(vui.vui_time_scale))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("vui_poc_proportional_to_timing_flag = " + QString::number(vui.vui_poc_proportional_to_timing_flag))));

    if(vui.vui_poc_proportional_to_timing_flag)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( vui_poc_proportional_to_timing_flag )"));
      pitem -> addChild(pitemSecond);
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("vui_num_ticks_poc_diff_one_minus1 = " + QString::number(vui.vui_num_ticks_poc_diff_one_minus1))));

      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("vui_hrd_parameters_present_flag = " + QString::number(vui.vui_hrd_parameters_present_flag))));

      if(vui.vui_hrd_parameters_present_flag)
      {
        QTreeWidgetItem *pitemThird = new QTreeWidgetItem(QStringList("if( vui_hrd_parameters_present_flag )"));
        pitemSecond -> addChild(pitemThird);

        QTreeWidgetItem *pitemHrd = new QTreeWidgetItem(QStringList("hrd_parameters(1, " + QString::number(maxNumSubLayersMinus1) + ")"));
        pitemThird -> addChild(pitemHrd);

        createHrdParameters(vui.hrd_parameters, 1, pitemHrd);
      }
    }
  }

  pVuiItem -> addChild(new QTreeWidgetItem(QStringList("bitstream_restriction_flag = " + QString::number(vui.bitstream_restriction_flag))));

  if(vui.bitstream_restriction_flag)
  {
    QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if( bitstream_restriction_flag )"));
    pVuiItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("tiles_fixed_structure_flag = " + QString::number(vui.tiles_fixed_structure_flag))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("motion_vectors_over_pic_boundaries_flag = " + QString::number(vui.motion_vectors_over_pic_boundaries_flag))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("restricted_ref_pic_lists_flag = " + QString::number(vui.restricted_ref_pic_lists_flag))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("min_spatial_segmentation_idc = " + QString::number(vui.min_spatial_segmentation_idc))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("max_bytes_per_pic_denom = " + QString::number(vui.max_bytes_per_pic_denom))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("max_bits_per_min_cu_denom = " + QString::number(vui.max_bits_per_min_cu_denom))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("log2_max_mv_length_horizontal = " + QString::number(vui.log2_max_mv_length_horizontal))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("log2_max_mv_length_vertical = " + QString::number(vui.log2_max_mv_length_vertical))));
  }
}


void SyntaxViewer::createHrdParameters(const HEVC::HrdParameters &hrd, uint8_t commonInfPresentFlag, QTreeWidgetItem *pHrdItem)
{
  if(commonInfPresentFlag)
  {
    QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if( commonInfPresentFlag )"));
    pHrdItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("nal_hrd_parameters_present_flag = " + QString::number(hrd.nal_hrd_parameters_present_flag))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("vcl_hrd_parameters_present_flag = " + QString::number(hrd.vcl_hrd_parameters_present_flag))));

    if(hrd.nal_hrd_parameters_present_flag || hrd.vcl_hrd_parameters_present_flag)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag )"));
      pitem -> addChild(pitemSecond);

      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("sub_pic_hrd_params_present_flag = " + QString::number(hrd.sub_pic_hrd_params_present_flag))));
      if(hrd.sub_pic_hrd_params_present_flag)
      {
        QTreeWidgetItem *pitemThird = new QTreeWidgetItem(QStringList("if (sub_pic_hrd_params_present_flag )"));
        pitemSecond -> addChild(pitemThird);

        pitemThird -> addChild(new QTreeWidgetItem(QStringList("tick_divisor_minus2 = " + QString::number(hrd.tick_divisor_minus2))));
        pitemThird -> addChild(new QTreeWidgetItem(QStringList("du_cpb_removal_delay_increment_length_minus1 = " + QString::number(hrd.du_cpb_removal_delay_increment_length_minus1))));
        pitemThird -> addChild(new QTreeWidgetItem(QStringList("sub_pic_cpb_params_in_pic_timing_sei_flag = " + QString::number(hrd.sub_pic_cpb_params_in_pic_timing_sei_flag))));
        pitemThird -> addChild(new QTreeWidgetItem(QStringList("dpb_output_delay_du_length_minus1 = " + QString::number(hrd.dpb_output_delay_du_length_minus1))));
      }

      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("bit_rate_scale = " + QString::number(hrd.bit_rate_scale))));
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("cpb_size_scale = " + QString::number(hrd.cpb_size_scale))));

      if(hrd.sub_pic_hrd_params_present_flag)
      {
        QTreeWidgetItem *pitemThird = new QTreeWidgetItem(QStringList("if (sub_pic_hrd_params_present_flag )"));
        pitemSecond -> addChild(pitemThird);

        pitemThird -> addChild(new QTreeWidgetItem(QStringList("cpb_size_du_scale = " + QString::number(hrd.cpb_size_du_scale))));
      }

      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("initial_cpb_removal_delay_length_minus1 = " + QString::number(hrd.initial_cpb_removal_delay_length_minus1))));
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("au_cpb_removal_delay_length_minus1 = " + QString::number(hrd.au_cpb_removal_delay_length_minus1))));
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("dpb_output_delay_length_minus1 = " + QString::number(hrd.dpb_output_delay_length_minus1))));
    }
  }

  if(hrd.fixed_pic_rate_general_flag.size() > 0)
  {
    QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("for( i = 0; i <= maxNumSubLayersMinus1; i++ )"));
    pHrdItem -> addChild(pitem);

    for(std::size_t i = 0; i < hrd.fixed_pic_rate_general_flag.size(); i++ ) 
    {
      pitem -> addChild(new QTreeWidgetItem(QStringList("fixed_pic_rate_general_flag[" + QString::number(i) + "] = " + QString::number(hrd.fixed_pic_rate_general_flag[i]))));

      if(!hrd.fixed_pic_rate_general_flag[i])
      {
        QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( !fixed_pic_rate_general_flag [" + QString::number(i) + "] )"));
        pitem -> addChild(pitemSecond);
        pitemSecond -> addChild(new QTreeWidgetItem(QStringList("fixed_pic_rate_within_cvs_flag[" + QString::number(i) + "] = " + QString::number(hrd.fixed_pic_rate_within_cvs_flag[i]))));
      }

      if(hrd.fixed_pic_rate_within_cvs_flag[i])
      {
        QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( fixed_pic_rate_within_cvs_flag[" + QString::number(i) + "] )"));
        pitem -> addChild(pitemSecond);
        pitemSecond -> addChild(new QTreeWidgetItem(QStringList("elemental_duration_in_tc_minus1[" + QString::number(i) + "] = " + QString::number(hrd.elemental_duration_in_tc_minus1[i]))));
      }
      else
      {
        QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( !fixed_pic_rate_within_cvs_flag[" + QString::number(i) + "] )"));
        pitem -> addChild(pitemSecond);
        pitemSecond -> addChild(new QTreeWidgetItem(QStringList("low_delay_hrd_flag[" + QString::number(i) + "] = " + QString::number(hrd.low_delay_hrd_flag[i]))));
      }

      if(!hrd.low_delay_hrd_flag[i])
      {
        QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( !low_delay_hrd_flag[" + QString::number(i) + "] )"));
        pitem -> addChild(pitemSecond);
        pitemSecond -> addChild(new QTreeWidgetItem(QStringList("cpb_cnt_minus1[" + QString::number(i) + "] = " + QString::number(hrd.cpb_cnt_minus1[i]))));
      }

      if(hrd.nal_hrd_parameters_present_flag)
      {
        QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( nal_hrd_parameters_present_flag )"));
        pitem -> addChild(pitemSecond);

        QTreeWidgetItem *pitemThird = new QTreeWidgetItem(QStringList("sub_layer_hrd_parameters(" + QString::number(i) + ")"));
        pitemSecond -> addChild(pitemThird);

        createSubLayerHrdParameters(hrd.nal_sub_layer_hrd_parameters[i], i, pitemThird);
      }
      if(hrd.vcl_hrd_parameters_present_flag)
      {
        QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( vcl_hrd_parameters_present_flag )"));
        pitem -> addChild(pitemSecond);

        QTreeWidgetItem *pitemThird = new QTreeWidgetItem(QStringList("sub_layer_hrd_parameters(" + QString::number(i) + ")"));
        pitemSecond -> addChild(pitemThird);

        createSubLayerHrdParameters(hrd.nal_sub_layer_hrd_parameters[i], i, pitemThird);
      }
    }
  }
}



void SyntaxViewer::createSubLayerHrdParameters(const HEVC::SubLayerHrdParameters &slhrd, uint8_t sub_pic_hrd_params_present_flag, QTreeWidgetItem *pSlhrdItem)
{
  QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("for( i = 0; i <= CpbCnt; i++ )"));
  pSlhrdItem -> addChild(pitem);

  for(std::size_t i=0; i<slhrd.bit_rate_value_minus1.size(); i++)
  {
    pitem -> addChild(new QTreeWidgetItem(QStringList("bit_rate_value_minus1[" + QString::number(i) + "] = " + QString::number(slhrd.bit_rate_value_minus1[i]))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("cpb_size_value_minus1[" + QString::number(i) + "] = " + QString::number(slhrd.cpb_size_value_minus1[i]))));

    if(sub_pic_hrd_params_present_flag)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( sub_pic_hrd_params_present_flag )"));
      pitem -> addChild(pitemSecond);

      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("cpb_size_du_value_minus1[" + QString::number(i) + "] = " + QString::number(slhrd.cpb_size_du_value_minus1[i]))));
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("bit_rate_du_value_minus1[" + QString::number(i) + "] = " + QString::number(slhrd.bit_rate_du_value_minus1[i]))));
    }

    pitem -> addChild(new QTreeWidgetItem(QStringList("cbr_flag[" + QString::number(i) + "] = " + QString::number(slhrd.cbr_flag[i]))));
  }
}


void SyntaxViewer::createShortTermRefPicSet(std::size_t stRpsIdx, const HEVC::ShortTermRefPicSet &rpset, std::size_t num_short_term_ref_pic_sets, const std::vector<HEVC::ShortTermRefPicSet> &refPicSets, QTreeWidgetItem *pStrpsItem)
{
//  HEVC::ShortTermRefPicSet rpset = refPicSets[stRpsIdx];

  if(stRpsIdx)
  {
    QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if( stRpsIdx )"));
    pStrpsItem -> addChild(pitem);
    pitem -> addChild(new QTreeWidgetItem(QStringList("inter_ref_pic_set_prediction_flag = " + QString::number(rpset.inter_ref_pic_set_prediction_flag))));
  }

  if(rpset.inter_ref_pic_set_prediction_flag)
  {
    QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if( inter_ref_pic_set_prediction_flag )"));
    pStrpsItem -> addChild(pitem);

    if(stRpsIdx == num_short_term_ref_pic_sets)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( stRpsIdx == num_short_term_ref_pic_sets )"));
      pitem -> addChild(pitemSecond);
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("delta_idx_minus1 = " + QString::number(rpset.delta_idx_minus1))));
    }

    pitem -> addChild(new QTreeWidgetItem(QStringList("delta_rps_sign = " + QString::number(rpset.delta_rps_sign))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("abs_delta_rps_minus1 = " + QString::number(rpset.abs_delta_rps_minus1))));

    std::size_t RefRpsIdx = stRpsIdx - (rpset.delta_idx_minus1 + 1);

    std::size_t NumDeltaPocs = 0;

    if(refPicSets[RefRpsIdx].inter_ref_pic_set_prediction_flag)
    {
      for(std::size_t i=0; i<refPicSets[RefRpsIdx].used_by_curr_pic_flag.size(); i++)
        if(refPicSets[RefRpsIdx].used_by_curr_pic_flag[i] || refPicSets[RefRpsIdx].use_delta_flag[i])
          NumDeltaPocs++;
    }
    else
      NumDeltaPocs = refPicSets[RefRpsIdx].num_negative_pics + refPicSets[RefRpsIdx].num_positive_pics;


    QTreeWidgetItem *pitemLoop = new QTreeWidgetItem(QStringList("for( j = 0; j <= NumDeltaPocs[ RefRpsIdx ]; j++ )"));
    pitem -> addChild(pitemLoop);

    for(std::size_t i=0; i<=NumDeltaPocs; i++ )
    {
      pitemLoop -> addChild(new QTreeWidgetItem(QStringList("used_by_curr_pic_flag[" + QString::number(i) + "] = " + QString::number(rpset.used_by_curr_pic_flag[i]))));

      if(!rpset.used_by_curr_pic_flag[i])
      {
        QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if( !used_by_curr_pic_flag[j] )"));
        pitemLoop -> addChild(pitem);
        pitem -> addChild(new QTreeWidgetItem(QStringList("use_delta_flag[" + QString::number(i) + "] = " + QString::number(rpset.use_delta_flag[i]))));

      }
    }
  }
  else 
  {
    QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if( !inter_ref_pic_set_prediction_flag )"));
    pStrpsItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("num_negative_pics = " + QString::number(rpset.num_negative_pics))));
    pitem -> addChild(new QTreeWidgetItem(QStringList("num_positive_pics = " + QString::number(rpset.num_positive_pics))));

    QTreeWidgetItem *pitemLoop = new QTreeWidgetItem(QStringList("for( i = 0; i < num_negative_pics; i++ )"));
    pitem -> addChild(pitemLoop);

    for(std::size_t i=0; i<rpset.num_negative_pics; i++) 
    {
      pitemLoop -> addChild(new QTreeWidgetItem(QStringList("delta_poc_s0_minus1[" + QString::number(i) + "] = " + QString::number(rpset.delta_poc_s0_minus1[i]))));
      pitemLoop -> addChild(new QTreeWidgetItem(QStringList("used_by_curr_pic_s0_flag[" + QString::number(i) + "] = " + QString::number(rpset.used_by_curr_pic_s0_flag[i]))));
    }

    pitemLoop = new QTreeWidgetItem(QStringList("for( i = 0; i < num_positive_pics; i++ )"));
    pitem -> addChild(pitemLoop);

    for(std::size_t i=0; i<rpset.num_positive_pics; i++) 
    {
      pitemLoop -> addChild(new QTreeWidgetItem(QStringList("delta_poc_s1_minus1[" + QString::number(i) + "] = " + QString::number(rpset.delta_poc_s1_minus1[i]))));
      pitemLoop -> addChild(new QTreeWidgetItem(QStringList("used_by_curr_pic_s1_flag[" + QString::number(i) + "] = " + QString::number(rpset.used_by_curr_pic_s1_flag[i]))));
    }
  }
}


void SyntaxViewer::createScalingListData(const HEVC::ScalingListData &scdata, QTreeWidgetItem *pScItem)
{
  QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("for( sizeId = 0; sizeId < 4; sizeId++ )"));
  pScItem -> addChild(pitem);


  QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("for( matrixId = 0; matrixId < ( ( sizeId == 3 ) ? 2 : 6 ); matrixId++ )"));
  pitem -> addChild(pitemSecond);


  for(std::size_t sizeId = 0; sizeId < 4; sizeId++)
  {
    for(std::size_t matrixId = 0; matrixId<((sizeId == 3)?2:6); matrixId++) 
    {
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("scaling_list_pred_mode_flag[" + QString::number(sizeId) + "][" + QString::number(matrixId) + "] = " + QString::number(scdata.scaling_list_pred_mode_flag[sizeId][matrixId]))));

      if(!scdata.scaling_list_pred_mode_flag[sizeId][matrixId])
      {
        pitem = new QTreeWidgetItem(QStringList("if( !scaling_list_pred_mode_flag[ sizeId ][ matrixId ] )"));
        pitemSecond -> addChild(pitem);
        pitem -> addChild(new QTreeWidgetItem(QStringList("scaling_list_pred_matrix_id_delta[" + QString::number(sizeId) + "][" + QString::number(matrixId) + "] = " + QString::number(scdata.scaling_list_pred_matrix_id_delta[sizeId][matrixId]))));
      }
      else
      {
        std::size_t coefNum = std::min(64, (1 << (4 + (sizeId << 1))));
        if(sizeId > 1)
        {
          pitem = new QTreeWidgetItem(QStringList("if( sizeId > 1 )"));
          pitemSecond -> addChild(pitem);
          pitem -> addChild(new QTreeWidgetItem(QStringList("scaling_list_dc_coef_minus8[" + QString::number(sizeId) + "][" + QString::number(matrixId) + "] = " + QString::number(scdata.scaling_list_dc_coef_minus8[sizeId-2][matrixId]))));
        }

        pitem = new QTreeWidgetItem(QStringList("for( i = 0; i < coefNum; i++ )"));
        pitemSecond -> addChild(pitem);

        for(std::size_t i = 0; i < coefNum; i++) 
        {
          pitem -> addChild(new QTreeWidgetItem(QStringList("scaling_list_delta_coef[" + QString::number(sizeId) + "][" + QString::number(matrixId) + "][" + QString::number(i) + "] = " + QString::number(scdata.scaling_list_delta_coef[sizeId][matrixId][i]))));
        }
      }
    }
  }
}


void SyntaxViewer::createRefPicListModification(const HEVC::RefPicListModification &rplModif, QTreeWidgetItem *pRplmItem)
{
  pRplmItem -> addChild(new QTreeWidgetItem(QStringList("ref_pic_list_modification_flag_l0 = " + QString::number(rplModif.ref_pic_list_modification_flag_l0))));

  if(rplModif.ref_pic_list_modification_flag_l0)
  {
    QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("for( i = 0; i <= num_ref_idx_l0_active_minus1; i++ )"));
    pRplmItem -> addChild(pitem);

    for(std::size_t i=0; i<rplModif.list_entry_l0.size(); i++)
      pitem -> addChild(new QTreeWidgetItem(QStringList("list_entry_l0[" + QString::number(i) + "] = " + QString::number(rplModif.list_entry_l0[i]))));
  }

  pRplmItem -> addChild(new QTreeWidgetItem(QStringList("ref_pic_list_modification_flag_l1 = " + QString::number(rplModif.ref_pic_list_modification_flag_l1))));

  if(rplModif.ref_pic_list_modification_flag_l1)
  {
    QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("for( i = 0; i <= num_ref_idx_l1_active_minus1; i++ )"));
    pRplmItem -> addChild(pitem);

    for(std::size_t i=0; i<rplModif.list_entry_l1.size(); i++)
      pitem -> addChild(new QTreeWidgetItem(QStringList("list_entry_l1[" + QString::number(i) + "] = " + QString::number(rplModif.list_entry_l1[i]))));
  }
}


void SyntaxViewer::createPredWeightTable(const HEVC::PredWeightTable &pwt, std::shared_ptr<HEVC::Slice> pSlice, QTreeWidgetItem *ppwtItem)
{
  std::shared_ptr<HEVC::PPS> ppps = m_ppsMap[pSlice -> slice_pic_parameter_set_id];
  if(!ppps)
    return ;

  std::shared_ptr<HEVC::SPS> psps = m_spsMap[ppps -> pps_seq_parameter_set_id];

  if(!psps)
    return ;

  ppwtItem -> addChild(new QTreeWidgetItem(QStringList("luma_log2_weight_denom = " + QString::number(pwt.luma_log2_weight_denom))));

  if(psps -> chroma_format_idc != 0)
  {
    QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if (psps -> chroma_format_idc)"));
    ppwtItem -> addChild(pitem);

    pitem -> addChild(new QTreeWidgetItem(QStringList("delta_chroma_log2_weight_denom = " + QString::number(pwt.delta_chroma_log2_weight_denom))));
  }

  QTreeWidgetItem *pitemLoop = new QTreeWidgetItem(QStringList("for(i=0; i<=num_ref_idx_l0_active_minus1; i++)"));
  ppwtItem -> addChild(pitemLoop);

  for(std::size_t i=0; i<=pSlice -> num_ref_idx_l0_active_minus1; i++)
    pitemLoop -> addChild(new QTreeWidgetItem(QStringList("luma_weight_l0_flag[" + QString::number(i) + "] = " + QString::number(pwt.luma_weight_l0_flag[i]))));

  if(psps -> chroma_format_idc != 0)
  {
    QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if (psps -> chroma_format_idc)"));
    ppwtItem -> addChild(pitem);

    pitemLoop = new QTreeWidgetItem(QStringList("for(i=0; i<=num_ref_idx_l0_active_minus1; i++)"));
    pitem -> addChild(pitemLoop);

    for(std::size_t i=0; i<=pSlice -> num_ref_idx_l0_active_minus1; i++)
      pitemLoop -> addChild(new QTreeWidgetItem(QStringList("chroma_weight_l0_flag[" + QString::number(i) + "] = " + QString::number(pwt.chroma_weight_l0_flag[i]))));
  }


  for(std::size_t i=0; i<=pSlice -> num_ref_idx_l0_active_minus1; i++)
  {
    pitemLoop = new QTreeWidgetItem(QStringList("for(i=0; i<=num_ref_idx_l0_active_minus1; i++)"));
    ppwtItem -> addChild(pitemLoop);

    if(pwt.luma_weight_l0_flag[i])
    {
      QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if (luma_weight_l0_flag[" + QString::number(i) +"])"));
      pitemLoop -> addChild(pitem);

      pitem -> addChild(new QTreeWidgetItem(QStringList("delta_luma_weight_l0[" + QString::number(i) + "] = " + QString::number(pwt.delta_luma_weight_l0[i]))));
      pitem -> addChild(new QTreeWidgetItem(QStringList("luma_offset_l0[" + QString::number(i) + "] = " + QString::number(pwt.luma_offset_l0[i]))));
    }
    if(pwt.chroma_weight_l0_flag[i])
    {
      QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if (chroma_weight_l0_flag[" + QString::number(i) +"])"));
      pitemLoop -> addChild(pitem);

      pitem -> addChild(new QTreeWidgetItem(QStringList("delta_chroma_weight_l0[" + QString::number(i) + "] = { " + QString::number(pwt.delta_chroma_weight_l0[i][0]) + ", " + QString::number(pwt.delta_chroma_weight_l0[i][1]) + " } ")));
      pitem -> addChild(new QTreeWidgetItem(QStringList("delta_chroma_offset_l0[" + QString::number(i) + "] = { " + QString::number(pwt.delta_chroma_offset_l0[i][0]) + ", " + QString::number(pwt.delta_chroma_offset_l0[i][1]) + " } ")));
    }
  }

  if(pSlice -> slice_type == SLICE_B)
  {
    QTreeWidgetItem *pitemBSlice = new QTreeWidgetItem(QStringList("slice_type == B"));
    ppwtItem -> addChild(pitemBSlice);

    QTreeWidgetItem *pitemLoop = new QTreeWidgetItem(QStringList("for(i=0; i<=num_ref_idx_l1_active_minus1; i++)"));
    pitemBSlice -> addChild(pitemLoop);

    for(std::size_t i=0; i<=pSlice -> num_ref_idx_l1_active_minus1; i++)
      pitemLoop -> addChild(new QTreeWidgetItem(QStringList("luma_weight_l1_flag[" + QString::number(i) + "] = " + QString::number(pwt.luma_weight_l1_flag[i]))));

    if(psps -> chroma_format_idc != 0)
    {
      QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if (psps -> chroma_format_idc)"));
      pitemBSlice -> addChild(pitem);

      pitemLoop = new QTreeWidgetItem(QStringList("for(i=0; i<=num_ref_idx_l1_active_minus1; i++)"));
      pitem -> addChild(pitemLoop);

      for(std::size_t i=0; i<=pSlice -> num_ref_idx_l1_active_minus1; i++)
        pitemLoop -> addChild(new QTreeWidgetItem(QStringList("chroma_weight_l1_flag[" + QString::number(i) + "] = " + QString::number(pwt.chroma_weight_l1_flag[i]))));
    }


    for(std::size_t i=0; i<=pSlice -> num_ref_idx_l1_active_minus1; i++)
    {
      pitemLoop = new QTreeWidgetItem(QStringList("for(i=0; i<=num_ref_idx_l1_active_minus1; i++)"));
      pitemBSlice -> addChild(pitemLoop);

      if(pwt.luma_weight_l1_flag[i])
      {
        QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if (luma_weight_l1_flag[" + QString::number(i) +"])"));
        pitemLoop -> addChild(pitem);

        pitem -> addChild(new QTreeWidgetItem(QStringList("delta_luma_weight_l1[" + QString::number(i) + "] = " + QString::number(pwt.delta_luma_weight_l1[i]))));
        pitem -> addChild(new QTreeWidgetItem(QStringList("luma_offset_l1[" + QString::number(i) + "] = " + QString::number(pwt.luma_offset_l1[i]))));
      }
      if(pwt.chroma_weight_l1_flag[i])
      {
        QTreeWidgetItem *pitem = new QTreeWidgetItem(QStringList("if (chroma_weight_l1_flag[" + QString::number(i) +"])"));
        pitemLoop -> addChild(pitem);

        pitem -> addChild(new QTreeWidgetItem(QStringList("delta_chroma_weight_l1[" + QString::number(i) + "] = { " + QString::number(pwt.delta_chroma_weight_l1[i][0]) + ", " + QString::number(pwt.delta_chroma_weight_l1[i][1]) + " } ")));
        pitem -> addChild(new QTreeWidgetItem(QStringList("delta_chroma_offset_l1[" + QString::number(i) + "] = { " + QString::number(pwt.delta_chroma_offset_l1[i][0]) + ", " + QString::number(pwt.delta_chroma_offset_l1[i][1]) + " } ")));
      }
    }
  } 

}



void SyntaxViewer::onItemCollapsed(QTreeWidgetItem * item)
{
  m_state.setActive(item, false);
}


void SyntaxViewer::onItemExpanded(QTreeWidgetItem * item)
{
  m_state.setActive(item, true);
}


void SyntaxViewer::updateItemsState()
{
  QTreeWidgetItemIterator itr(this);

  for(;*itr; itr++) 
    (*itr)-> setExpanded(m_state.isActive((*itr)));
}


bool SyntaxViewer::SyntaxViewerState::isActive(const QTreeWidgetItem *pitem) const
{
  std::map<QString, bool>::const_iterator itr = m_state.begin();

  for(;itr != m_state.end(); itr++)
  {
    if(itr -> first == name(pitem))
      break;
  }

  if(itr != m_state.end())
    return itr -> second;

  return true;
}


void SyntaxViewer::SyntaxViewerState::setActive(const QTreeWidgetItem *pitem, bool isActive)
{
  m_state[name(pitem)] = isActive;
}


QString SyntaxViewer::SyntaxViewerState::name(const QTreeWidgetItem *pitem) const
{
  QString name;
  while(pitem)
  {
    if(name.isEmpty())
      name = pitem -> text(0);
    else
      name = pitem -> text(0) + "::" + name;

    pitem = pitem -> parent();
  }

  return name;
}
