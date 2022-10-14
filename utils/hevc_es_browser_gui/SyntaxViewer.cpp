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
  switch(pNalU -> m_nalHeader.type)
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

        createHrdParameters(pVPS -> hrd_parameters[i], pVPS -> cprms_present_flag[i], pitemThird);
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

  if(pSlice -> m_nalHeader.type >= HEVC::NAL_BLA_W_LP && pSlice -> m_nalHeader.type <= HEVC::NAL_IRAP_VCL23)
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
    bool IdrPicFlag = pSlice -> m_nalHeader.type == HEVC::NAL_IDR_W_RADL || pSlice -> m_nalHeader.type == HEVC::NAL_IDR_N_LP;

    if(!IdrPicFlag)
    {
      pitemDepend = new QTreeWidgetItem(QStringList("if (nal_unit_type != IDR_W_RADL && nal_unit_type != IDR_N_LP)"));
      pitem -> addChild(pitemDepend);

      pitemDepend -> addChild(new QTreeWidgetItem(QStringList("slice_pic_order_cnt_lsb = " + QString::number(pSlice -> slice_pic_order_cnt_lsb))));

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

        if((pSlice -> collocated_from_l0_flag && pSlice -> num_ref_idx_l0_active_minus1) ||
            (!pSlice -> collocated_from_l0_flag && pSlice -> num_ref_idx_l1_active_minus1))
        {
          QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if ((collocated_from_l0_flag && num_ref_idx_l0_active_minus1 > 0 ) || (!collocated_from_l0_flag && num_ref_idx_l1_active_minus1 > 0))"));
          pitemThird -> addChild(pitemSecond);

          pitemSecond -> addChild(new QTreeWidgetItem(QStringList("collocated_ref_idx = " + QString::number(pSlice -> collocated_ref_idx))));
        }
      }

      if((pPPS -> weighted_pred_flag && pSlice -> slice_type == SLICE_P) ||
        (pPPS -> weighted_bipred_flag && pSlice -> slice_type == SLICE_B))
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
      for(std::size_t j=0; j<pSEI -> sei_message[i].num_payload_size_ff_bytes; j++)
      {
        pitemSecond -> addChild(new QTreeWidgetItem(QStringList("0xFF")));
        payloadSize += 255;
      }
    }

    pitem -> addChild(new QTreeWidgetItem(QStringList("last_payload_size_byte = " + QString::number(pSEI -> sei_message[i].last_payload_size_byte))));

    payloadSize += pSEI -> sei_message[i].last_payload_size_byte;
    switch(payloadType)
    {
      case HEVC::SeiMessage::DECODED_PICTURE_HASH:
      {
        std::shared_ptr<HEVC::DecodedPictureHash> pdecPictHash = std::dynamic_pointer_cast<HEVC::DecodedPictureHash>(pSEI -> sei_message[i].sei_payload);

        QTreeWidgetItem *pitemDecPictHash = new QTreeWidgetItem(QStringList("decoded_picture_hash(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemDecPictHash);
        createDecodedPictureHash(pdecPictHash, pitemDecPictHash);
        break;
      }

      case HEVC::SeiMessage::USER_DATA_UNREGISTERED:
      {
        std::shared_ptr<HEVC::UserDataUnregistered> pSeiMessage = std::dynamic_pointer_cast<HEVC::UserDataUnregistered>(pSEI -> sei_message[i].sei_payload);

        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("user_data_unregistered(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createUserDataUnregistered(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::SCENE_INFO:
      {
        std::shared_ptr<HEVC::SceneInfo> pSeiMessage = std::dynamic_pointer_cast<HEVC::SceneInfo>(pSEI -> sei_message[i].sei_payload);

        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("scene_info(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createSceneInfo(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::FULL_FRAME_SNAPSHOT:
      {
        std::shared_ptr<HEVC::FullFrameSnapshot> pSeiMessage = std::dynamic_pointer_cast<HEVC::FullFrameSnapshot>(pSEI -> sei_message[i].sei_payload);

        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("picture_snapshot(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createFullFrameSnapshot(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::PROGRESSIVE_REFINEMENT_SEGMENT_START:
      {
        std::shared_ptr<HEVC::ProgressiveRefinementSegmentStart> pSeiMessage = std::dynamic_pointer_cast<HEVC::ProgressiveRefinementSegmentStart>(pSEI -> sei_message[i].sei_payload);

        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("progressive_refinement_segment_start(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createProgressiveRefinementSegmentStart(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::PROGRESSIVE_REFINEMENT_SEGMENT_END:
      {
        std::shared_ptr<HEVC::ProgressiveRefinementSegmentEnd> pSeiMessage = std::dynamic_pointer_cast<HEVC::ProgressiveRefinementSegmentEnd>(pSEI -> sei_message[i].sei_payload);

        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("progressive_refinement_segment_end(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createProgressiveRefinementSegmentEnd(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::BUFFERING_PERIOD:
      {
        std::shared_ptr<HEVC::BufferingPeriod> pSeiMessage = std::dynamic_pointer_cast<HEVC::BufferingPeriod>(pSEI -> sei_message[i].sei_payload);
        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("buffering_period(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createBufferingPeriod(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::FILLER_PAYLOAD:
      {
        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("filler_payload(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        break;
      }

      case HEVC::SeiMessage::PICTURE_TIMING:
      {
        std::shared_ptr<HEVC::PicTiming> pSeiMessage = std::dynamic_pointer_cast<HEVC::PicTiming>(pSEI -> sei_message[i].sei_payload);
        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("pic_timing(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createPicTiming(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::RECOVERY_POINT:
      {
        std::shared_ptr<HEVC::RecoveryPoint> pSeiMessage = std::dynamic_pointer_cast<HEVC::RecoveryPoint>(pSEI -> sei_message[i].sei_payload);
        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("recovery_point(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createRecoveryPoint(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::TONE_MAPPING_INFO:
      {
        std::shared_ptr<HEVC::ToneMapping> pSeiMessage = std::dynamic_pointer_cast<HEVC::ToneMapping>(pSEI -> sei_message[i].sei_payload);
        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("tone_mapping_info(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createToneMapping(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::FRAME_PACKING:
      {
        std::shared_ptr<HEVC::FramePacking> pSeiMessage = std::dynamic_pointer_cast<HEVC::FramePacking>(pSEI -> sei_message[i].sei_payload);
        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("frame_packing_arrangement(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createFramePacking(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::DISPLAY_ORIENTATION:
      {
        std::shared_ptr<HEVC::DisplayOrientation> pSeiMessage = std::dynamic_pointer_cast<HEVC::DisplayOrientation>(pSEI -> sei_message[i].sei_payload);
        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("display_orientation(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createDisplayOrientation(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::SOP_DESCRIPTION:
      {
        std::shared_ptr<HEVC::SOPDescription> pSeiMessage = std::dynamic_pointer_cast<HEVC::SOPDescription>(pSEI -> sei_message[i].sei_payload);
        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("structure_of_pictures_info(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createSOPDescription(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::ACTIVE_PARAMETER_SETS:
      {
        std::shared_ptr<HEVC::ActiveParameterSets> pSeiMessage = std::dynamic_pointer_cast<HEVC::ActiveParameterSets>(pSEI -> sei_message[i].sei_payload);

        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("active_parameter_sets(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createActiveParameterSets(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::TEMPORAL_LEVEL0_INDEX:
      {
        std::shared_ptr<HEVC::TemporalLevel0Index> pSeiMessage = std::dynamic_pointer_cast<HEVC::TemporalLevel0Index>(pSEI -> sei_message[i].sei_payload);

        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("temporal_sub_layer_zero_index(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createTemporalLevel0Index(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::REGION_REFRESH_INFO:
      {
        std::shared_ptr<HEVC::RegionRefreshInfo> pSeiMessage = std::dynamic_pointer_cast<HEVC::RegionRefreshInfo>(pSEI -> sei_message[i].sei_payload);

        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("region_refresh_info(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createRegionRefreshInfo(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::TIME_CODE:
      {
        std::shared_ptr<HEVC::TimeCode> pSeiMessage = std::dynamic_pointer_cast<HEVC::TimeCode>(pSEI -> sei_message[i].sei_payload);

        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("time_code(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createTimeCode(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::MASTERING_DISPLAY_INFO:
      {
        std::shared_ptr<HEVC::MasteringDisplayInfo> pSeiMessage = std::dynamic_pointer_cast<HEVC::MasteringDisplayInfo>(pSEI -> sei_message[i].sei_payload);

        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("mastering_display_info(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createMasteringDisplayInfo(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::SEGM_RECT_FRAME_PACKING:
      {
        std::shared_ptr<HEVC::SegmRectFramePacking> pSeiMessage = std::dynamic_pointer_cast<HEVC::SegmRectFramePacking>(pSEI -> sei_message[i].sei_payload);

        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("segm_rect_frame_packing(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createSegmRectFramePacking(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::KNEE_FUNCTION_INFO:
      {
        std::shared_ptr<HEVC::KneeFunctionInfo> pSeiMessage = std::dynamic_pointer_cast<HEVC::KneeFunctionInfo>(pSEI -> sei_message[i].sei_payload);

        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("knee_function_info(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createKneeFunctionInfo(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::CHROMA_RESAMPLING_FILTER_HINT:
      {
        std::shared_ptr<HEVC::ChromaResamplingFilterHint> pSeiMessage = std::dynamic_pointer_cast<HEVC::ChromaResamplingFilterHint>(pSEI -> sei_message[i].sei_payload);

        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("chroma_resampling_filter_hint(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createChromaResamplingFilterHint(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::COLOUR_REMAPPING_INFO:
      {
        std::shared_ptr<HEVC::ColourRemappingInfo> pSeiMessage = std::dynamic_pointer_cast<HEVC::ColourRemappingInfo>(pSEI -> sei_message[i].sei_payload);

        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("colour_remapping_info(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createColourRemappingInfo(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::CONTENT_LIGHT_LEVEL_INFO:
      {
        std::shared_ptr<HEVC::ContentLightLevelInfo> pSeiMessage = std::dynamic_pointer_cast<HEVC::ContentLightLevelInfo>(pSEI -> sei_message[i].sei_payload);

        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("content_light_level_info(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createContentLightLevelInfo(pSeiMessage, pitemSei);
        break;
      }

      case HEVC::SeiMessage::ALTERNATIVE_TRANSFER_CHARACTERISTICS:
      {
        std::shared_ptr<HEVC::AlternativeTransferCharacteristics> pSeiMessage = std::dynamic_pointer_cast<HEVC::AlternativeTransferCharacteristics>(pSEI -> sei_message[i].sei_payload);

        QTreeWidgetItem *pitemSei = new QTreeWidgetItem(QStringList("alternative_transfer_characteristics(" + QString::number(payloadSize) + ")"));
        pitem -> addChild(pitemSei);
        createAlternativeTransferCharacteristics(pSeiMessage, pitemSei);
        break;
      }

      default:
        pitem -> addChild(new QTreeWidgetItem(QStringList("sei_payload(" + QString::number(payloadType) + ", " + QString::number(payloadSize) + ")")));
    }
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
    }

    pitem -> addChild(new QTreeWidgetItem(QStringList("vui_hrd_parameters_present_flag = " + QString::number(vui.vui_hrd_parameters_present_flag))));

    if(vui.vui_hrd_parameters_present_flag)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( vui_hrd_parameters_present_flag )"));
      pitem -> addChild(pitemSecond);

      QTreeWidgetItem *pitemHrd = new QTreeWidgetItem(QStringList("hrd_parameters(1, " + QString::number(maxNumSubLayersMinus1) + ")"));
      pitemSecond -> addChild(pitemHrd);

      createHrdParameters(vui.hrd_parameters, 1, pitemHrd);
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

        createSubLayerHrdParameters(hrd.nal_sub_layer_hrd_parameters[i], hrd.sub_pic_hrd_params_present_flag, pitemThird);
      }
      if(hrd.vcl_hrd_parameters_present_flag)
      {
        QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( vcl_hrd_parameters_present_flag )"));
        pitem -> addChild(pitemSecond);

        QTreeWidgetItem *pitemThird = new QTreeWidgetItem(QStringList("sub_layer_hrd_parameters(" + QString::number(i) + ")"));
        pitemSecond -> addChild(pitemThird);

        createSubLayerHrdParameters(hrd.vcl_sub_layer_hrd_parameters[i], hrd.sub_pic_hrd_params_present_flag, pitemThird);
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

void SyntaxViewer::createDecodedPictureHash(std::shared_ptr<HEVC::DecodedPictureHash> pDecPictHash, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("hash_type = " + QString::number(pDecPictHash->hash_type))));

  QTreeWidgetItem *ploop = new QTreeWidgetItem(QStringList("for( cIdx = 0; cIdx < ( chroma_format_idc = = 0 ? 1 : 3 ); cIdx++ )"));
  pItem -> addChild(ploop);

  QTreeWidgetItem *pif = new QTreeWidgetItem(QStringList("if(hash_type = " + QString::number(pDecPictHash->hash_type) + ") "));
  ploop -> addChild(pif);


  if(pDecPictHash->hash_type == 0)
  {
    for(std::size_t i=0; i<pDecPictHash->picture_md5.size(); i++)
    {
      QString str;
      for(std::size_t j=0; j<16; j++)
        str += QString("%1").arg(pDecPictHash->picture_md5[i][j], 2, 16, QChar('0')).toUpper();
      pif -> addChild(new QTreeWidgetItem(QStringList("picture_md5[" + QString::number(i) + "] = " + str)));
    }
  }
  if(pDecPictHash->hash_type == 1)
  {
    for(std::size_t i=0; i<pDecPictHash->picture_crc.size(); i++)
    {
      pif -> addChild(new QTreeWidgetItem(QStringList("picture_crc[" + QString::number(i) + "] = " + QString::number(pDecPictHash -> picture_crc[i]))));
    }
  }
  else
  {
    for(std::size_t i=0; i<pDecPictHash->picture_crc.size(); i++)
    {
      pif -> addChild(new QTreeWidgetItem(QStringList("picture_checksum[" + QString::number(i) + "] = " + QString::number(pDecPictHash -> picture_checksum[i]))));
    }
  }
}


void SyntaxViewer::createMasteringDisplayInfo(std::shared_ptr<HEVC::MasteringDisplayInfo> pSei, QTreeWidgetItem *pItem)
{
  QTreeWidgetItem *pitemLoop = new QTreeWidgetItem(QStringList("for( i = 0; i < 3 ; i++ )"));
  pItem -> addChild(pitemLoop);


  for (uint32_t i = 0; i < 3; i++)
  {
    pitemLoop -> addChild(new QTreeWidgetItem(QStringList("display_primary_x[" + QString::number(i) + "] = " + QString::number(pSei -> display_primary_x[i]))));
    pitemLoop -> addChild(new QTreeWidgetItem(QStringList("display_primary_y[" + QString::number(i) + "] = " + QString::number(pSei -> display_primary_y[i]))));
  }

  pItem -> addChild(new QTreeWidgetItem(QStringList("white_point_x = " + QString::number(pSei -> white_point_x))));
  pItem -> addChild(new QTreeWidgetItem(QStringList("white_point_y = " + QString::number(pSei -> white_point_y))));

  pItem -> addChild(new QTreeWidgetItem(QStringList("max_display_mastering_luminance = " + QString::number(pSei -> max_display_mastering_luminance))));
  pItem -> addChild(new QTreeWidgetItem(QStringList("min_display_mastering_luminance = " + QString::number(pSei -> min_display_mastering_luminance))));
}


void SyntaxViewer::createActiveParameterSets(std::shared_ptr<HEVC::ActiveParameterSets> pSeiPayload, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("active_video_parameter_set_id = " + QString::number(pSeiPayload -> active_video_parameter_set_id))));
  pItem -> addChild(new QTreeWidgetItem(QStringList("self_contained_cvs_flag = " + QString::number(pSeiPayload -> self_contained_cvs_flag))));
  pItem -> addChild(new QTreeWidgetItem(QStringList("no_parameter_set_update_flag = " + QString::number(pSeiPayload -> no_parameter_set_update_flag))));
  pItem -> addChild(new QTreeWidgetItem(QStringList("num_sps_ids_minus1 = " + QString::number(pSeiPayload -> num_sps_ids_minus1))));

  QString str = "active_seq_parameter_set_id = {\n\t";
  for (uint32_t i = 0; i < pSeiPayload -> num_sps_ids_minus1; i++)
  {
    str += QString::number(pSeiPayload -> active_seq_parameter_set_id[i]) + ",  ";
    if((i + 1) % 8)
      str += "\n\t";
  }

  str += QString::number(pSeiPayload -> active_seq_parameter_set_id[pSeiPayload -> num_sps_ids_minus1]) + "\n}";
  pItem -> addChild(new QTreeWidgetItem(QStringList(str)));
}


void SyntaxViewer::createUserDataUnregistered(std::shared_ptr<HEVC::UserDataUnregistered> pSei, QTreeWidgetItem *pItem)
{
  QString str =
    QString("%1").arg(pSei -> uuid_iso_iec_11578[0], 2, 16, QChar('0')).toLower() +
    QString("%1").arg(pSei -> uuid_iso_iec_11578[1], 2, 16, QChar('0')).toLower() +
    QString("%1").arg(pSei -> uuid_iso_iec_11578[2], 2, 16, QChar('0')).toLower() +
    QString("%1").arg(pSei -> uuid_iso_iec_11578[3], 2, 16, QChar('0')).toLower() +
    "-" +
    QString("%1").arg(pSei -> uuid_iso_iec_11578[4], 2, 16, QChar('0')).toLower() +
    QString("%1").arg(pSei -> uuid_iso_iec_11578[5], 2, 16, QChar('0')).toLower() +
    "-" +
    QString("%1").arg(pSei -> uuid_iso_iec_11578[6], 2, 16, QChar('0')).toLower() +
    QString("%1").arg(pSei -> uuid_iso_iec_11578[7], 2, 16, QChar('0')).toLower() +
    "-" +
    QString("%1").arg(pSei -> uuid_iso_iec_11578[8], 2, 16, QChar('0')).toLower() +
    QString("%1").arg(pSei -> uuid_iso_iec_11578[9], 2, 16, QChar('0')).toLower() +
    "-" +
    QString("%1").arg(pSei -> uuid_iso_iec_11578[10], 2, 16, QChar('0')).toLower() +
    QString("%1").arg(pSei -> uuid_iso_iec_11578[11], 2, 16, QChar('0')).toLower() +
    QString("%1").arg(pSei -> uuid_iso_iec_11578[12], 2, 16, QChar('0')).toLower() +
    QString("%1").arg(pSei -> uuid_iso_iec_11578[13], 2, 16, QChar('0')).toLower() +
    QString("%1").arg(pSei -> uuid_iso_iec_11578[14], 2, 16, QChar('0')).toLower() +
    QString("%1").arg(pSei -> uuid_iso_iec_11578[15], 2, 16, QChar('0')).toLower();

  pItem -> addChild(new QTreeWidgetItem(QStringList("uuid_iso_iec_11578 = " + str)));


  if(pSei -> user_data_payload_byte.empty())
  {
    str = "user_data_payload_byte = { }";
  }
  else
  {
    str = "user_data_payload_byte = {\n\t";
    for (int i = 0; i < pSei -> user_data_payload_byte.size() - 1; i++)
    {
      str += QString("%1").arg(pSei -> user_data_payload_byte[i], 2, 16, QChar('0')).toLower();
      if((i + 1) % 16 == 0)
        str += "\n\t";
    }
    str += QString::number(pSei -> user_data_payload_byte[pSei -> user_data_payload_byte.size() - 1], 16) + "\n}";
  }

  pItem -> addChild(new QTreeWidgetItem(QStringList(str)));
}


void SyntaxViewer::createBufferingPeriod(std::shared_ptr<HEVC::BufferingPeriod> pSei, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("bp_seq_parameter_set_id = " + QString::number(pSei->bp_seq_parameter_set_id))));

  std::shared_ptr<HEVC::SPS> psps = m_spsMap[pSei -> bp_seq_parameter_set_id];
  if(!psps)
    return;

  if(!psps->vui_parameters.hrd_parameters.sub_pic_hrd_params_present_flag)
  {
    QTreeWidgetItem *pitemIrap = new QTreeWidgetItem(QStringList("if( !sub_pic_hrd_params_present_flag )"));
    pItem -> addChild(pitemIrap);
    pitemIrap -> addChild(new QTreeWidgetItem(QStringList("irap_cpb_params_present_flag = " + QString::number(pSei->irap_cpb_params_present_flag))));
  }

  if(pSei -> irap_cpb_params_present_flag)
  {
    QTreeWidgetItem *pitemIrap = new QTreeWidgetItem(QStringList("if( irap_cpb_params_present_flag )"));
    pItem -> addChild(pitemIrap);
    pitemIrap -> addChild(new QTreeWidgetItem(QStringList("cpb_delay_offset = " + QString::number(pSei->cpb_delay_offset))));
    pitemIrap -> addChild(new QTreeWidgetItem(QStringList("dpb_delay_offset = " + QString::number(pSei->dpb_delay_offset))));
  }

  pItem -> addChild(new QTreeWidgetItem(QStringList("concatenation_flag = " + QString::number(pSei->concatenation_flag))));
  pItem -> addChild(new QTreeWidgetItem(QStringList("au_cpb_removal_delay_delta_minus1 = " + QString::number(pSei->au_cpb_removal_delay_delta_minus1))));

  bool NalHrdBpPresentFlag = psps->vui_parameters.hrd_parameters.nal_hrd_parameters_present_flag;
  if(NalHrdBpPresentFlag)
  {
    QTreeWidgetItem *pitemBpPresentFlag= new QTreeWidgetItem(QStringList("if( NalHrdBpPresentFlag )"));
    pItem -> addChild(pitemBpPresentFlag);

    QTreeWidgetItem *ploop = new QTreeWidgetItem(QStringList("for( i = 0; i <= CpbCnt; i++ )"));
    pitemBpPresentFlag -> addChild(ploop);

    for(std::size_t i=0; i<pSei->nal_initial_cpb_removal_delay.size(); i++)
    {
      ploop -> addChild(new QTreeWidgetItem(QStringList("nal_initial_cpb_removal_delay[" + QString::number(i) + "] = " + QString::number(pSei -> nal_initial_cpb_removal_delay[i]))));
      ploop -> addChild(new QTreeWidgetItem(QStringList("nal_initial_cpb_removal_offset[" + QString::number(i) + "] = " + QString::number(pSei -> nal_initial_cpb_removal_offset[i]))));

      if(psps->vui_parameters.hrd_parameters.sub_pic_hrd_params_present_flag || pSei->irap_cpb_params_present_flag)
      {
        QTreeWidgetItem *pitemAlt = new QTreeWidgetItem(QStringList("if( sub_pic_hrd_params_present_flag | | irap_cpb_params_present_flag )"));
        ploop -> addChild(pitemAlt);

        pitemAlt -> addChild(new QTreeWidgetItem(QStringList("nal_initial_alt_cpb_removal_delay[" + QString::number(i) + "] = " + QString::number(pSei -> nal_initial_alt_cpb_removal_delay[i]))));
        pitemAlt -> addChild(new QTreeWidgetItem(QStringList("nal_initial_alt_cpb_removal_offset[" + QString::number(i) + "] = " + QString::number(pSei -> nal_initial_alt_cpb_removal_offset[i]))));
      }

    }
  }

  bool VclHrdBpPresentFlag = psps->vui_parameters.hrd_parameters.vcl_hrd_parameters_present_flag;
  if(VclHrdBpPresentFlag)
  {
    QTreeWidgetItem *pitemBpPresentFlag= new QTreeWidgetItem(QStringList("if( VclHrdBpPresentFlag )"));
    pItem -> addChild(pitemBpPresentFlag);

    QTreeWidgetItem *ploop = new QTreeWidgetItem(QStringList("for( i = 0; i <= CpbCnt; i++ )"));
    pitemBpPresentFlag -> addChild(ploop);

    for(std::size_t i=0; i<pSei->vcl_initial_cpb_removal_delay.size(); i++)
    {
      ploop -> addChild(new QTreeWidgetItem(QStringList("vcl_initial_cpb_removal_delay[" + QString::number(i) + "] = " + QString::number(pSei -> vcl_initial_cpb_removal_delay[i]))));
      ploop -> addChild(new QTreeWidgetItem(QStringList("vcl_initial_cpb_removal_offset[" + QString::number(i) + "] = " + QString::number(pSei -> vcl_initial_cpb_removal_offset[i]))));

      if(psps->vui_parameters.hrd_parameters.sub_pic_hrd_params_present_flag || pSei->irap_cpb_params_present_flag)
      {
        QTreeWidgetItem *pitemAlt = new QTreeWidgetItem(QStringList("if( sub_pic_hrd_params_present_flag | | irap_cpb_params_present_flag )"));
        ploop -> addChild(pitemAlt);

        pitemAlt -> addChild(new QTreeWidgetItem(QStringList("vcl_initial_alt_cpb_removal_delay[" + QString::number(i) + "] = " + QString::number(pSei -> vcl_initial_alt_cpb_removal_delay[i]))));
        pitemAlt -> addChild(new QTreeWidgetItem(QStringList("vcl_initial_alt_cpb_removal_offset[" + QString::number(i) + "] = " + QString::number(pSei -> vcl_initial_alt_cpb_removal_offset[i]))));
      }
    }
  }
}


void SyntaxViewer::createPicTiming(std::shared_ptr<HEVC::PicTiming> pSei, QTreeWidgetItem *pItem)
{
  std::shared_ptr<HEVC::SPS> psps;

  if(m_spsMap.size())
    psps = m_spsMap.begin() -> second;

  if(!psps)
    return;


  if(psps->vui_parameters.frame_field_info_present_flag)
  {
    QTreeWidgetItem *pitemField = new QTreeWidgetItem(QStringList("if( frame_field_info_present_flag )"));
    pItem -> addChild(pitemField);

    pitemField -> addChild(new QTreeWidgetItem(QStringList("pic_struct = " + QString::number(pSei->pic_struct))));
    pitemField -> addChild(new QTreeWidgetItem(QStringList("source_scan_type = " + QString::number(pSei->source_scan_type))));
    pitemField -> addChild(new QTreeWidgetItem(QStringList("duplicate_flag = " + QString::number(pSei->duplicate_flag))));
  }

  bool CpbDpbDelaysPresentFlag = psps->vui_parameters.hrd_parameters.nal_hrd_parameters_present_flag ||
                                  psps->vui_parameters.hrd_parameters.vcl_hrd_parameters_present_flag;

  if(CpbDpbDelaysPresentFlag)
  {
    QTreeWidgetItem *pitemDpbDelays = new QTreeWidgetItem(QStringList("if( CpbDpbDelaysPresentFlag )"));
    pItem -> addChild(pitemDpbDelays);

    pitemDpbDelays -> addChild(new QTreeWidgetItem(QStringList("au_cpb_removal_delay_minus1 = " + QString::number(pSei->au_cpb_removal_delay_minus1))));
    pitemDpbDelays -> addChild(new QTreeWidgetItem(QStringList("pic_dpb_output_delay = " + QString::number(pSei->pic_dpb_output_delay))));

    if(psps->vui_parameters.hrd_parameters.sub_pic_hrd_params_present_flag)
    {
      QTreeWidgetItem *pitemDuDelay = new QTreeWidgetItem(QStringList("if( CpbDpbDelaysPresentFlag )"));
      pitemDpbDelays -> addChild(pitemDuDelay);

      pitemDuDelay -> addChild(new QTreeWidgetItem(QStringList("pic_dpb_output_du_delay = " + QString::number(pSei->pic_dpb_output_du_delay))));
    }

    if(psps->vui_parameters.hrd_parameters.sub_pic_hrd_params_present_flag &&
        psps->vui_parameters.hrd_parameters.sub_pic_cpb_params_in_pic_timing_sei_flag)
    {
      QTreeWidgetItem *pitemIf = new QTreeWidgetItem(QStringList("if( CpbDpbDelaysPresentFlag )"));
      pitemDpbDelays -> addChild(pitemIf);

      pitemIf -> addChild(new QTreeWidgetItem(QStringList("num_decoding_units_minus1 = " + QString::number(pSei->num_decoding_units_minus1))));
      pitemIf -> addChild(new QTreeWidgetItem(QStringList("du_common_cpb_removal_delay_flag = " + QString::number(pSei->du_common_cpb_removal_delay_flag))));

      if(pSei -> du_common_cpb_removal_delay_flag)
      {
        QTreeWidgetItem *pitemDuCommCpb = new QTreeWidgetItem(QStringList("if( CpbDpbDelaysPresentFlag )"));
        pitemIf -> addChild(pitemDuCommCpb);

        pitemDuCommCpb -> addChild(new QTreeWidgetItem(QStringList("du_common_cpb_removal_delay_increment_minus1 = " + QString::number(pSei->num_decoding_units_minus1))));
      }

      QTreeWidgetItem *ploop = new QTreeWidgetItem(QStringList("for( i = 0; i <= num_decoding_units_minus1; i++ )"));
      pitemIf -> addChild(ploop);

      for(std::size_t i=0; i<=pSei -> num_decoding_units_minus1; i++)
      {
        ploop -> addChild(new QTreeWidgetItem(QStringList("num_nalus_in_du_minus1[" + QString::number(i) + "] = " + QString::number(pSei -> num_nalus_in_du_minus1[i]))));

        if(!pSei -> du_common_cpb_removal_delay_flag && i<pSei -> num_decoding_units_minus1)
        {
          QTreeWidgetItem *pitemDuCommCpb = new QTreeWidgetItem(QStringList("if( !du_common_cpb_removal_delay_flag && i < num_decoding_units_minus1 )"));
          ploop -> addChild(pitemDuCommCpb);

          pitemDuCommCpb -> addChild(new QTreeWidgetItem(QStringList("du_cpb_removal_delay_increment_minus1[" + QString::number(i) + "] = " + QString::number(pSei -> du_cpb_removal_delay_increment_minus1[i]))));
        }
      }
    }
  }
}


void SyntaxViewer::createRecoveryPoint(std::shared_ptr<HEVC::RecoveryPoint> pSei, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("recovery_poc_cnt = " + QString::number(pSei->recovery_poc_cnt))));
  pItem -> addChild(new QTreeWidgetItem(QStringList("exact_match_flag = " + QString::number(pSei->exact_match_flag))));
  pItem -> addChild(new QTreeWidgetItem(QStringList("broken_link_flag = " + QString::number(pSei->broken_link_flag))));
}


void SyntaxViewer::createContentLightLevelInfo(std::shared_ptr<HEVC::ContentLightLevelInfo> pSeiPayload, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("max_content_light_level = " + QString::number(pSeiPayload->max_content_light_level))));
  pItem -> addChild(new QTreeWidgetItem(QStringList("max_pic_average_light_level = " + QString::number(pSeiPayload->max_pic_average_light_level))));
}

void SyntaxViewer::createAlternativeTransferCharacteristics(std::shared_ptr<HEVC::AlternativeTransferCharacteristics> pSeiPayload, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("alternative_transfer_characteristics = " + QString::number(pSeiPayload->alternative_transfer_characteristics))));
}


void SyntaxViewer::createFramePacking(std::shared_ptr<HEVC::FramePacking> pSei, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("frame_packing_arrangement_id = " + QString::number(pSei->frame_packing_arrangement_id))));
  pItem -> addChild(new QTreeWidgetItem(QStringList("frame_packing_arrangement_cancel_flag = " + QString::number(pSei->frame_packing_arrangement_cancel_flag))));

  if(!pSei -> frame_packing_arrangement_cancel_flag)
  {
    QTreeWidgetItem *pitemIf = new QTreeWidgetItem(QStringList("if( !frame_packing_arrangement_cancel_flag )"));
    pItem -> addChild(pitemIf);

    pitemIf -> addChild(new QTreeWidgetItem(QStringList("frame_packing_arrangement_type = " + QString::number(pSei->frame_packing_arrangement_type))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("quincunx_sampling_flag = " + QString::number(pSei->quincunx_sampling_flag))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("content_interpretation_type = " + QString::number(pSei->content_interpretation_type))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("spatial_flipping_flag = " + QString::number(pSei->spatial_flipping_flag))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("frame0_flipped_flag = " + QString::number(pSei->frame0_flipped_flag))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("field_views_flag = " + QString::number(pSei->field_views_flag))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("current_frame_is_frame0_flag = " + QString::number(pSei->current_frame_is_frame0_flag))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("frame0_self_contained_flag = " + QString::number(pSei->frame0_self_contained_flag))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("frame1_self_contained_flag = " + QString::number(pSei->frame1_self_contained_flag))));

    if(!pSei -> quincunx_sampling_flag && pSei -> frame_packing_arrangement_type != 5)
    {
      QTreeWidgetItem *pitemGridPos = new QTreeWidgetItem(QStringList("if( !quincunx_sampling_flag && frame_packing_arrangement_type != 5 )"));
      pitemIf -> addChild(pitemGridPos);

      pitemGridPos -> addChild(new QTreeWidgetItem(QStringList("frame0_grid_position_x = " + QString::number(pSei->frame0_grid_position_x))));
      pitemGridPos -> addChild(new QTreeWidgetItem(QStringList("frame0_grid_position_y = " + QString::number(pSei->frame0_grid_position_y))));
      pitemGridPos -> addChild(new QTreeWidgetItem(QStringList("frame1_grid_position_x = " + QString::number(pSei->frame1_grid_position_x))));
      pitemGridPos -> addChild(new QTreeWidgetItem(QStringList("frame1_grid_position_y = " + QString::number(pSei->frame1_grid_position_y))));
    }

    pitemIf -> addChild(new QTreeWidgetItem(QStringList("frame_packing_arrangement_reserved_byte = " + QString::number(pSei->frame_packing_arrangement_reserved_byte))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("frame_packing_arrangement_persistence_flag = " + QString::number(pSei->frame_packing_arrangement_persistence_flag))));

  }

  pItem -> addChild(new QTreeWidgetItem(QStringList("upsampled_aspect_ratio_flag = " + QString::number(pSei->upsampled_aspect_ratio_flag))));
}


void SyntaxViewer::createDisplayOrientation(std::shared_ptr<HEVC::DisplayOrientation> pSei, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("display_orientation_cancel_flag = " + QString::number(pSei->display_orientation_cancel_flag))));
  if(!pSei -> display_orientation_cancel_flag)
  {
    QTreeWidgetItem *pitemIf = new QTreeWidgetItem(QStringList("if( !display_orientation_cancel_flag )"));
    pItem -> addChild(pitemIf);

    pitemIf -> addChild(new QTreeWidgetItem(QStringList("hor_flip = " + QString::number(pSei->hor_flip))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("ver_flip = " + QString::number(pSei->ver_flip))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("anticlockwise_rotation = " + QString::number(pSei->anticlockwise_rotation))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("display_orientation_persistence_flag = " + QString::number(pSei->display_orientation_persistence_flag))));
  }
}


void SyntaxViewer::createToneMapping(std::shared_ptr<HEVC::ToneMapping> pSei, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("tone_map_id = " + QString::number(pSei->tone_map_id))));
  pItem -> addChild(new QTreeWidgetItem(QStringList("tone_map_cancel_flag = " + QString::number(pSei->tone_map_cancel_flag))));

  if(!pSei -> tone_map_cancel_flag)
  {
    QTreeWidgetItem *pitemIf = new QTreeWidgetItem(QStringList("if( !tone_map_cancel_flag )"));
    pItem -> addChild(pitemIf);

    pitemIf -> addChild(new QTreeWidgetItem(QStringList("tone_map_persistence_flag = " + QString::number(pSei->tone_map_persistence_flag))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("coded_data_bit_depth = " + QString::number(pSei->coded_data_bit_depth))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("target_bit_depth = " + QString::number(pSei->target_bit_depth))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("tone_map_model_id = " + QString::number(pSei->tone_map_model_id))));

    if(pSei -> tone_map_model_id == 0)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( tone_map_model_id = = 0 )"));
      pItem -> addChild(pitemSecond);

      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("min_value = " + QString::number(pSei->min_value))));
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("max_value = " + QString::number(pSei->max_value))));
    }
    else if(pSei -> tone_map_model_id == 1)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( tone_map_model_id = = 1 )"));
      pItem -> addChild(pitemSecond);

      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("sigmoid_midpoint = " + QString::number(pSei->sigmoid_midpoint))));
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("sigmoid_width = " + QString::number(pSei->sigmoid_width))));
    }
    else if(pSei -> tone_map_model_id == 2)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( tone_map_model_id = = 2 )"));
      pItem -> addChild(pitemSecond);

      QTreeWidgetItem *ploop = new QTreeWidgetItem(QStringList("for( i = 0; i < ( 1 << target_bit_depth ); i++ )"));
      pitemSecond -> addChild(ploop);

      for(std::size_t i = 0; i<(1 << pSei->target_bit_depth); i++)
      {
        ploop -> addChild(new QTreeWidgetItem(QStringList("start_of_coded_interval[" + QString::number(i) + "] = " + QString::number(pSei -> start_of_coded_interval[i]))));
      }

    }
    else if(pSei -> tone_map_model_id == 3)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( tone_map_model_id = = 3 )"));
      pItem -> addChild(pitemSecond);

      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("num_pivots = " + QString::number(pSei->num_pivots))));

      QTreeWidgetItem *ploop = new QTreeWidgetItem(QStringList("for( i = 0; i < num_pivots; i++ )"));
      pitemSecond -> addChild(ploop);

      for(std::size_t i=0; i<pSei -> num_pivots; i++)
      {
        ploop -> addChild(new QTreeWidgetItem(QStringList("coded_pivot_value[" + QString::number(i) + "] = " + QString::number(pSei -> coded_pivot_value[i]))));
        ploop -> addChild(new QTreeWidgetItem(QStringList("target_pivot_value[" + QString::number(i) + "] = " + QString::number(pSei -> target_pivot_value[i]))));
      }
    }
    else if(pSei -> tone_map_model_id == 4)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( tone_map_model_id = = 4 )"));
      pItem -> addChild(pitemSecond);

      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("camera_iso_speed_idc = " + QString::number(pSei->camera_iso_speed_idc))));
      if(pSei -> camera_iso_speed_idc == 255)
      {
        QTreeWidgetItem *pitemThird = new QTreeWidgetItem(QStringList("if( camera_iso_speed_idc = = EXTENDED_ISO )"));
        pitemSecond -> addChild(pitemThird);

        pitemThird -> addChild(new QTreeWidgetItem(QStringList("camera_iso_speed_value = " + QString::number(pSei->camera_iso_speed_value))));
      }

      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("exposure_index_idc = " + QString::number(pSei->exposure_index_idc))));
      if(pSei -> exposure_index_idc == 255)
      {
        QTreeWidgetItem *pitemThird = new QTreeWidgetItem(QStringList("if( exposure_index_idc = = EXTENDED_ISO )"));
        pitemSecond -> addChild(pitemThird);

        pitemThird -> addChild(new QTreeWidgetItem(QStringList("exposure_index_value = " + QString::number(pSei->exposure_index_value))));
      }

      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("exposure_compensation_value_sign_flag = " + QString::number(pSei->exposure_compensation_value_sign_flag))));
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("exposure_compensation_value_numerator = " + QString::number(pSei->exposure_compensation_value_numerator))));
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("exposure_compensation_value_denom_idc = " + QString::number(pSei->exposure_compensation_value_denom_idc))));
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("ref_screen_luminance_white = " + QString::number(pSei->ref_screen_luminance_white))));
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("extended_range_white_level = " + QString::number(pSei->extended_range_white_level))));
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("nominal_black_level_code_value = " + QString::number(pSei->nominal_black_level_code_value))));
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("nominal_white_level_code_value = " + QString::number(pSei->nominal_white_level_code_value))));
      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("extended_white_level_code_value = " + QString::number(pSei->extended_white_level_code_value))));
    }
  }
}


void SyntaxViewer::createSOPDescription(std::shared_ptr<HEVC::SOPDescription> pSei, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("sop_seq_parameter_set_id = " + QString::number(pSei->sop_seq_parameter_set_id))));
  pItem -> addChild(new QTreeWidgetItem(QStringList("num_entries_in_sop_minus1 = " + QString::number(pSei->num_entries_in_sop_minus1))));

  QTreeWidgetItem *ploop = new QTreeWidgetItem(QStringList("for( i = 0; i <= num_entries_in_sop_minus1; i++ )"));
  pItem -> addChild(ploop);

  for(std::size_t i=0; i<=pSei -> num_entries_in_sop_minus1; i++)
  {
    ploop -> addChild(new QTreeWidgetItem(QStringList("sop_vcl_nut[" + QString::number(i) + "] = " + QString::number(pSei -> sop_vcl_nut[i]))));
    ploop -> addChild(new QTreeWidgetItem(QStringList("sop_temporal_id[" + QString::number(i) + "] = " + QString::number(pSei -> sop_temporal_id[i]))));

    if(pSei -> sop_vcl_nut[i] != 19 && pSei -> sop_vcl_nut[i] != 20)
    {
      QTreeWidgetItem *pitemIf = new QTreeWidgetItem(QStringList("if( sop_vcl_nut[ i ] != IDR_W_RADL && sop_vcl_nut[ i ] != IDR_N_LP )"));
      ploop -> addChild(pitemIf);

      pitemIf -> addChild(new QTreeWidgetItem(QStringList("sop_short_term_rps_idx[" + QString::number(i) + "] = " + QString::number(pSei -> sop_short_term_rps_idx[i]))));
    }

    if(i > 0)
    {
      QTreeWidgetItem *pitemIf = new QTreeWidgetItem(QStringList("if( i > 0 )"));
      ploop -> addChild(pitemIf);

      pitemIf -> addChild(new QTreeWidgetItem(QStringList("sop_poc_delta[" + QString::number(i) + "] = " + QString::number(pSei -> sop_poc_delta[i]))));
    }
  }
}


void SyntaxViewer::createTemporalLevel0Index(std::shared_ptr<HEVC::TemporalLevel0Index> pSeiPayload, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("temporal_sub_layer_zero_idx = " + QString::number(pSeiPayload->temporal_sub_layer_zero_idx))));
  pItem -> addChild(new QTreeWidgetItem(QStringList("irap_pic_id = " + QString::number(pSeiPayload->irap_pic_id))));
}

void SyntaxViewer::createSegmRectFramePacking(std::shared_ptr<HEVC::SegmRectFramePacking> pSei, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("segmented_rect_frame_packing_arrangement_cancel_flag = " + QString::number(pSei->segmented_rect_frame_packing_arrangement_cancel_flag))));

  if(!pSei -> segmented_rect_frame_packing_arrangement_cancel_flag)
  {
    QTreeWidgetItem *pitemIf = new QTreeWidgetItem(QStringList("if( !segmented_rect_frame_packing_arrangement_cancel_flag )"));
    pItem -> addChild(pitemIf);

    pitemIf -> addChild(new QTreeWidgetItem(QStringList("segmented_rect_content_interpretation_type = " + QString::number(pSei->segmented_rect_content_interpretation_type))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("segmented_rect_frame_packing_arrangement_persistence = " + QString::number(pSei->segmented_rect_frame_packing_arrangement_persistence))));
  }
}


void SyntaxViewer::createTimeCode(std::shared_ptr<HEVC::TimeCode> pSei, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("num_clock_ts = " + QString::number(pSei->num_clock_ts))));

  if(pSei -> num_clock_ts > 0)
  {
    QTreeWidgetItem *ploop = new QTreeWidgetItem(QStringList("for( i = 0; i < num_clock_ts; i++ )"));
    pItem -> addChild(ploop);

    for(std::size_t i=0; i<pSei -> num_clock_ts; i++)
    {
      ploop -> addChild(new QTreeWidgetItem(QStringList("clock_time_stamp_flag[" + QString::number(i) + "] = " + QString::number(pSei -> clock_time_stamp_flag[i]))));

      if(pSei -> clock_time_stamp_flag[i])
      {
        QTreeWidgetItem *pitemIf = new QTreeWidgetItem(QStringList("if( clock_time_stamp_flag[" + QString::number(i) + "] )"));
        ploop -> addChild(pitemIf);

        pitemIf -> addChild(new QTreeWidgetItem(QStringList("nuit_field_based_flag[" + QString::number(i) + "] = " + QString::number(pSei -> nuit_field_based_flag[i]))));
        pitemIf -> addChild(new QTreeWidgetItem(QStringList("counting_type[" + QString::number(i) + "] = " + QString::number(pSei -> counting_type[i]))));
        pitemIf -> addChild(new QTreeWidgetItem(QStringList("full_timestamp_flag[" + QString::number(i) + "] = " + QString::number(pSei -> full_timestamp_flag[i]))));
        pitemIf -> addChild(new QTreeWidgetItem(QStringList("discontinuity_flag[" + QString::number(i) + "] = " + QString::number(pSei -> discontinuity_flag[i]))));
        pitemIf -> addChild(new QTreeWidgetItem(QStringList("cnt_dropped_flag[" + QString::number(i) + "] = " + QString::number(pSei -> cnt_dropped_flag[i]))));
        pitemIf -> addChild(new QTreeWidgetItem(QStringList("n_frames[" + QString::number(i) + "] = " + QString::number(pSei -> n_frames[i]))));


        if(pSei -> full_timestamp_flag[i])
        {
          QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( full_timestamp_flag[" + QString::number(i) + "] )"));
          pitemIf -> addChild(pitemSecond);

          pitemSecond -> addChild(new QTreeWidgetItem(QStringList("seconds_value[" + QString::number(i) + "] = " + QString::number(pSei -> seconds_value[i]))));
          pitemSecond -> addChild(new QTreeWidgetItem(QStringList("minutes_value[" + QString::number(i) + "] = " + QString::number(pSei -> minutes_value[i]))));
          pitemSecond -> addChild(new QTreeWidgetItem(QStringList("hours_value[" + QString::number(i) + "] = " + QString::number(pSei -> hours_value[i]))));
        }
        else
        {
          QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( !full_timestamp_flag[" + QString::number(i) + "] )"));
          pitemIf -> addChild(pitemSecond);

          pitemSecond -> addChild(new QTreeWidgetItem(QStringList("seconds_flag[" + QString::number(i) + "] = " + QString::number(pSei -> seconds_flag[i]))));

          if(pSei -> seconds_flag[i])
          {
            QTreeWidgetItem *pitemSeconds = new QTreeWidgetItem(QStringList("if( seconds_flag[" + QString::number(i) + "] )"));
            pitemSecond -> addChild(pitemSeconds);

            pitemSeconds -> addChild(new QTreeWidgetItem(QStringList("seconds_value[" + QString::number(i) + "] = " + QString::number(pSei -> seconds_value[i]))));
            pitemSeconds -> addChild(new QTreeWidgetItem(QStringList("minutes_flag[" + QString::number(i) + "] = " + QString::number(pSei -> minutes_flag[i]))));

            if(pSei -> minutes_flag[i])
            {
              QTreeWidgetItem *pitemMinuts = new QTreeWidgetItem(QStringList("if( minutes_flag[" + QString::number(i) + "] )"));
              pitemSeconds -> addChild(pitemMinuts);

              pitemMinuts -> addChild(new QTreeWidgetItem(QStringList("minutes_value[" + QString::number(i) + "] = " + QString::number(pSei -> minutes_value[i]))));
              pitemMinuts -> addChild(new QTreeWidgetItem(QStringList("hours_flag[" + QString::number(i) + "] = " + QString::number(pSei -> hours_flag[i]))));

              if(pSei -> hours_flag[i])
              {
                QTreeWidgetItem *pitemHours = new QTreeWidgetItem(QStringList("if( hours_flag[" + QString::number(i) + "] )"));
                pitemMinuts -> addChild(pitemHours);

                pitemHours -> addChild(new QTreeWidgetItem(QStringList("hours_value[" + QString::number(i) + "] = " + QString::number(pSei -> hours_value[i]))));
              }
            }
          }
        }

        pitemIf -> addChild(new QTreeWidgetItem(QStringList("time_offset_length[" + QString::number(i) + "] = " + QString::number(pSei -> time_offset_length[i]))));
        if(pSei -> time_offset_length[i])
        {
          QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( time_offset_length[" + QString::number(i) + "] )"));
          pitemIf -> addChild(pitemSecond);

          pitemSecond -> addChild(new QTreeWidgetItem(QStringList("time_offset_value[" + QString::number(i) + "] = " + QString::number(pSei -> time_offset_value[i]))));
        }
      }
    }
  }
}


void SyntaxViewer::createKneeFunctionInfo(std::shared_ptr<HEVC::KneeFunctionInfo> pSei, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("knee_function_id = " + QString::number(pSei->knee_function_id))));
  pItem -> addChild(new QTreeWidgetItem(QStringList("knee_function_cancel_flag = " + QString::number(pSei->knee_function_cancel_flag))));

  if(!pSei -> knee_function_cancel_flag)
  {
    QTreeWidgetItem *pitemIf = new QTreeWidgetItem(QStringList("if( !knee_function_cancel_flag )"));
    pItem -> addChild(pitemIf);

    pitemIf -> addChild(new QTreeWidgetItem(QStringList("knee_function_persistence_flag = " + QString::number(pSei -> knee_function_persistence_flag))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("input_d_range = " + QString::number(pSei -> input_d_range))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("input_disp_luminance = " + QString::number(pSei -> input_disp_luminance))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("output_d_range = " + QString::number(pSei -> output_d_range))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("output_disp_luminance = " + QString::number(pSei -> output_disp_luminance))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("num_knee_points_minus1 = " + QString::number(pSei -> num_knee_points_minus1))));

    QTreeWidgetItem *ploop = new QTreeWidgetItem(QStringList("for( i = 0; i <= num_knee_points_minus1; i++ )"));
    pitemIf -> addChild(ploop);

    for(std::size_t i=0; i<=pSei -> num_knee_points_minus1; i++)
    {
      ploop -> addChild(new QTreeWidgetItem(QStringList("input_knee_point[" + QString::number(i) + "] = " + QString::number(pSei -> input_knee_point[i]))));
      ploop -> addChild(new QTreeWidgetItem(QStringList("output_knee_point[" + QString::number(i) + "] = " + QString::number(pSei -> output_knee_point[i]))));
    }
  }
}


void SyntaxViewer::createChromaResamplingFilterHint(std::shared_ptr<HEVC::ChromaResamplingFilterHint> pSei, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("ver_chroma_filter_idc = " + QString::number(pSei->ver_chroma_filter_idc))));
  pItem -> addChild(new QTreeWidgetItem(QStringList("hor_chroma_filter_idc = " + QString::number(pSei->hor_chroma_filter_idc))));
  pItem -> addChild(new QTreeWidgetItem(QStringList("ver_filtering_field_processing_flag = " + QString::number(pSei->ver_filtering_field_processing_flag))));

  if(pSei -> ver_chroma_filter_idc == 1 || pSei -> hor_chroma_filter_idc == 1)
  {
    QTreeWidgetItem *pitemIf = new QTreeWidgetItem(QStringList("if( ver_chroma_filter_idc == 1 || hor_chroma_filter_idc == 1 )"));
    pItem -> addChild(pitemIf);

    pitemIf -> addChild(new QTreeWidgetItem(QStringList("target_format_idc = " + QString::number(pSei -> target_format_idc))));

    if(pSei -> ver_chroma_filter_idc == 1)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( ver_chroma_filter_idc == 1 )"));
      pitemIf -> addChild(pitemIf);

      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("num_vertical_filters = " + QString::number(pSei -> num_vertical_filters))));

      if(pSei -> num_vertical_filters)
      {
        QTreeWidgetItem *ploop = new QTreeWidgetItem(QStringList("for( i = 0; i < num_vertical_filters; i++ )"));
        pitemSecond -> addChild(ploop);

        for(std::size_t i=0; i<pSei -> num_vertical_filters; i++)
        {
          ploop -> addChild(new QTreeWidgetItem(QStringList("ver_tap_length_minus_1[" + QString::number(i) + "] = " + QString::number(pSei -> ver_tap_length_minus_1[i]))));

          QString str = "ver_filter_coeff[" + QString::number(i) + "] = {\n\t";
          for(std::size_t j=0; j<pSei -> ver_tap_length_minus_1[i]; j++)
          {
            str += QString::number(pSei -> ver_filter_coeff[i][j]) + ", ";
            if((i+1) % 8 == 0)
              str += "\n\t";
          }
          str += QString::number(pSei -> ver_filter_coeff[i][pSei -> ver_tap_length_minus_1[i]]) + " \n}";

          pitemSecond -> addChild(new QTreeWidgetItem(QStringList(str)));
        }
      }
    }

    if(pSei -> hor_chroma_filter_idc == 1)
    {
      QTreeWidgetItem *pitemSecond = new QTreeWidgetItem(QStringList("if( hor_chroma_filter_idc == 1 )"));
      pitemIf -> addChild(pitemIf);

      pitemSecond -> addChild(new QTreeWidgetItem(QStringList("num_horizontal_filters = " + QString::number(pSei -> num_horizontal_filters))));

      if(pSei -> num_horizontal_filters)
      {
        QTreeWidgetItem *ploop = new QTreeWidgetItem(QStringList("for( i = 0; i < num_horizontal_filters; i++ )"));
        pitemSecond -> addChild(ploop);

        for(std::size_t i=0; i<pSei -> num_horizontal_filters; i++)
        {
          ploop -> addChild(new QTreeWidgetItem(QStringList("hor_tap_length_minus_1[" + QString::number(i) + "] = " + QString::number(pSei -> hor_tap_length_minus_1[i]))));

          QString str = "hor_filter_coeff[" + QString::number(i) + "] = {\n\t";
          for(std::size_t j=0; j<pSei -> hor_tap_length_minus_1[i]; j++)
          {
            str += QString::number(pSei -> hor_filter_coeff[i][j]) + ", ";
            if((i+1) % 8 == 0)
              str += "\n\t";
          }
          str += QString::number(pSei -> hor_filter_coeff[i][pSei -> hor_tap_length_minus_1[i]]) + " \n}";

          pitemSecond -> addChild(new QTreeWidgetItem(QStringList(str)));
        }
      }
    }
  }
}


void SyntaxViewer::createColourRemappingInfo(std::shared_ptr<HEVC::ColourRemappingInfo> pSei, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("colour_remap_id = " + QString::number(pSei->colour_remap_id))));
  pItem -> addChild(new QTreeWidgetItem(QStringList("colour_remap_cancel_flag = " + QString::number(pSei->colour_remap_cancel_flag))));

  if(!pSei -> colour_remap_cancel_flag)
  {
    QTreeWidgetItem *pitemIf = new QTreeWidgetItem(QStringList("if( !colour_remap_cancel_flag )"));
    pItem -> addChild(pitemIf);

    pitemIf -> addChild(new QTreeWidgetItem(QStringList("colour_remap_persistence_flag = " + QString::number(pSei -> colour_remap_persistence_flag))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("colour_remap_video_signal_info_present_flag = " + QString::number(pSei -> colour_remap_video_signal_info_present_flag))));

    if(pSei -> colour_remap_video_signal_info_present_flag)
    {
      QTreeWidgetItem *pitemIfSecond = new QTreeWidgetItem(QStringList("if( colour_remap_video_signal_info_present_flag )"));
      pitemIf -> addChild(pitemIfSecond);

      pitemIfSecond -> addChild(new QTreeWidgetItem(QStringList("colour_remap_full_range_flag = " + QString::number(pSei -> colour_remap_full_range_flag))));
      pitemIfSecond -> addChild(new QTreeWidgetItem(QStringList("colour_remap_primaries = " + QString::number(pSei -> colour_remap_primaries))));
      pitemIfSecond -> addChild(new QTreeWidgetItem(QStringList("colour_remap_transfer_function = " + QString::number(pSei -> colour_remap_transfer_function))));
      pitemIfSecond -> addChild(new QTreeWidgetItem(QStringList("colour_remap_matrix_coefficients = " + QString::number(pSei -> colour_remap_matrix_coefficients))));
    }
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("colour_remap_input_bit_depth = " + QString::number(pSei -> colour_remap_input_bit_depth))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("colour_remap_bit_depth = " + QString::number(pSei -> colour_remap_bit_depth))));

    QTreeWidgetItem *ploop = new QTreeWidgetItem(QStringList("for( i = 0; i < 3; i++ )"));
    pitemIf -> addChild(ploop);

    for(std::size_t i=0 ; i<3 ; i++)
    {
      ploop -> addChild(new QTreeWidgetItem(QStringList("pre_lut_num_val_minus1[" + QString::number(i) + "] = " + QString::number(pSei -> pre_lut_num_val_minus1[i]))));

      if(pSei -> pre_lut_num_val_minus1[i] > 0)
      {
        QTreeWidgetItem *pitemIfSecond = new QTreeWidgetItem(QStringList("if( pre_lut_num_val_minus1[" + QString::number(i) + "] > 0 )"));
        ploop -> addChild(pitemIfSecond);

        QTreeWidgetItem *ploopSecond = new QTreeWidgetItem(QStringList("for( j = 0; j <= pre_lut_num_val_minus1[" + QString::number(i) + "]; j++ )"));
        pitemIfSecond -> addChild(ploopSecond);

        for (std::size_t j=0 ; j<=pSei -> pre_lut_num_val_minus1[i]; j++)
        {
          ploopSecond -> addChild(new QTreeWidgetItem(QStringList("pre_lut_coded_value[" + QString::number(i) + "][" + QString::number(j) + "] = " + QString::number(pSei -> pre_lut_coded_value[i][j]))));
          ploopSecond -> addChild(new QTreeWidgetItem(QStringList("pre_lut_target_value[" + QString::number(i) + "][" + QString::number(j) + "] = " + QString::number(pSei -> pre_lut_target_value[i][j]))));
        }
      }
    }

    pitemIf -> addChild(new QTreeWidgetItem(QStringList("colour_remap_matrix_present_flag = " + QString::number(pSei -> colour_remap_matrix_present_flag))));

    if(pSei -> colour_remap_matrix_present_flag)
    {
      QTreeWidgetItem *pitemIfSecond = new QTreeWidgetItem(QStringList("if( colour_remap_matrix_present_flag )"));
      pitemIf -> addChild(pitemIfSecond);

      pitemIfSecond -> addChild(new QTreeWidgetItem(QStringList("log2_matrix_denom = " + QString::number(pSei -> log2_matrix_denom))));

      QTreeWidgetItem *ploop = new QTreeWidgetItem(QStringList("for( i = 0; i < 3; i++ )"));
      pitemIfSecond -> addChild(ploop);

      for (std::size_t i=0 ; i<3 ; i++)
      {
        QString str = "colour_remap_coeffs[" + QString::number(i) + "] = { ";
        for (std::size_t j=0 ; j<2 ; j++)
        {
          str += QString::number(pSei -> colour_remap_coeffs[i][j]) + ", ";
        }
        str += QString::number(pSei -> colour_remap_coeffs[i][2]) + " }";

        ploop -> addChild(new QTreeWidgetItem(QStringList(str)));
      }
    }

    ploop = new QTreeWidgetItem(QStringList("for( i = 0; i < 3; i++ )"));
    pitemIf -> addChild(ploop);

    for(std::size_t i=0 ; i<3 ; i++)
    {
      ploop -> addChild(new QTreeWidgetItem(QStringList("post_lut_num_val_minus1[" + QString::number(i) + "] = " + QString::number(pSei -> post_lut_num_val_minus1[i]))));

      if(pSei -> post_lut_num_val_minus1[i] > 0)
      {
        QTreeWidgetItem *pitemIfSecond = new QTreeWidgetItem(QStringList("if( post_lut_num_val_minus1[" + QString::number(i) + "] > 0 )"));
        ploop -> addChild(pitemIfSecond);

        QTreeWidgetItem *ploopSecond = new QTreeWidgetItem(QStringList("for( j = 0; j <= post_lut_num_val_minus1[" + QString::number(i) + "]; j++ )"));
        pitemIfSecond -> addChild(ploopSecond);

        for (std::size_t j=0 ; j<=pSei -> post_lut_num_val_minus1[i]; j++)
        {
          ploopSecond -> addChild(new QTreeWidgetItem(QStringList("post_lut_coded_value[" + QString::number(i) + "][" + QString::number(j) + "] = " + QString::number(pSei -> post_lut_coded_value[i][j]))));
          ploopSecond -> addChild(new QTreeWidgetItem(QStringList("post_lut_target_value[" + QString::number(i) + "][" + QString::number(j) + "] = " + QString::number(pSei -> post_lut_target_value[i][j]))));
        }
      }
    }
  }
}


void SyntaxViewer::createSceneInfo(std::shared_ptr<HEVC::SceneInfo> pSei, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("scene_info_present_flag = " + QString::number(pSei->scene_info_present_flag))));

  if(pSei -> scene_info_present_flag)
  {
    QTreeWidgetItem *pitemIf = new QTreeWidgetItem(QStringList("if( scene_info_present_flag )"));
    pItem -> addChild(pitemIf);

    pitemIf -> addChild(new QTreeWidgetItem(QStringList("prev_scene_id_valid_flag = " + QString::number(pSei -> prev_scene_id_valid_flag))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("scene_id = " + QString::number(pSei -> scene_id))));
    pitemIf -> addChild(new QTreeWidgetItem(QStringList("scene_transition_type = " + QString::number(pSei -> scene_transition_type))));

    if(pSei -> scene_transition_type > 3)
    {
      QTreeWidgetItem *pitemIfSecond = new QTreeWidgetItem(QStringList("if( scene_transition_type > 3 )"));
      pitemIf -> addChild(pitemIfSecond);

      pitemIfSecond -> addChild(new QTreeWidgetItem(QStringList("second_scene_id = " + QString::number(pSei -> second_scene_id))));
    }
  }
}

void SyntaxViewer::createProgressiveRefinementSegmentStart(std::shared_ptr<HEVC::ProgressiveRefinementSegmentStart> pSei, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("progressive_refinement_id = " + QString::number(pSei->progressive_refinement_id))));
  pItem -> addChild(new QTreeWidgetItem(QStringList("pic_order_cnt_delta = " + QString::number(pSei->pic_order_cnt_delta))));
}


void SyntaxViewer::createProgressiveRefinementSegmentEnd(std::shared_ptr<HEVC::ProgressiveRefinementSegmentEnd> pSei, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("progressive_refinement_id = " + QString::number(pSei->progressive_refinement_id))));
}


void SyntaxViewer::createFullFrameSnapshot(std::shared_ptr<HEVC::FullFrameSnapshot> pSei, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("snapshot_id = " + QString::number(pSei->snapshot_id))));
}


void SyntaxViewer::createRegionRefreshInfo(std::shared_ptr<HEVC::RegionRefreshInfo> pSei, QTreeWidgetItem *pItem)
{
  pItem -> addChild(new QTreeWidgetItem(QStringList("refreshed_region_flag = " + QString::number(pSei->refreshed_region_flag))));
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
  std::map<QString, bool>::const_iterator itr = m_state.find(name(pitem));
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
