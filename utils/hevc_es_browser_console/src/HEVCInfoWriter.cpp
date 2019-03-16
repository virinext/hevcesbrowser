#include "HEVCInfoWriter.h"

#include <HevcUtils.h>
#include <ConvToString.h>

#include <iostream>

using namespace HEVC;

void HEVCInfoWriter::onNALUnit(std::shared_ptr<HEVC::NALUnit> pNALUnit, const HEVC::Parser::Info *pInfo)
{
  NALUInfo nalUInfo;

  nalUInfo.m_pNALUnit = pNALUnit;
  nalUInfo.m_info = *pInfo;
  m_nalus.push_back(nalUInfo);
}

void HEVCInfoWriter::write(std::ostream &out)
{
  out << "Syntax elements (count = " << m_nalus.size() << "):" << std::endl;
  for(std::size_t i=0; i<m_nalus.size(); i++)
  {
    writeNALHeader(m_nalus[i], out);

    switch(m_nalus[i].m_pNALUnit -> m_nalUnitType)
    {
      case NAL_VPS:
      {
        std::shared_ptr<HEVC::VPS> pVPS = std::dynamic_pointer_cast<HEVC::VPS>(m_nalus[i].m_pNALUnit);
        writeVPS(pVPS, out);
        m_vpsMap[pVPS -> vps_video_parameter_set_id] = pVPS;
        break;
      }

      case NAL_SPS:
      {
        std::shared_ptr<HEVC::SPS> pSPS = std::dynamic_pointer_cast<HEVC::SPS>(m_nalus[i].m_pNALUnit);
        writeSPS(pSPS, out);
        m_spsMap[pSPS -> sps_seq_parameter_set_id] = pSPS;
        break;
      }

      case NAL_PPS:
      {
        std::shared_ptr<HEVC::PPS> pPPS = std::dynamic_pointer_cast<HEVC::PPS>(m_nalus[i].m_pNALUnit);
        writePPS(pPPS, out);
        m_ppsMap[pPPS -> pps_pic_parameter_set_id] = pPPS;
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
        writeSlice(std::dynamic_pointer_cast<HEVC::Slice>(m_nalus[i].m_pNALUnit), out);
        break;
      }

      case NAL_AUD:
      {
        std::shared_ptr<HEVC::AUD> pAUD = std::dynamic_pointer_cast<HEVC::AUD>(m_nalus[i].m_pNALUnit);
        writeAUD(pAUD, out);
        break;
      }

      case NAL_SEI_SUFFIX:
      case NAL_SEI_PREFIX:
      {
        writeSEI(std::dynamic_pointer_cast<HEVC::SEI>(m_nalus[i].m_pNALUnit), out);
        break;

      }
    };
    out << std::endl;
  }
}


void HEVCInfoWriter::writeNALHeader(const NALUInfo& naluInfo, std::ostream &out)
{
  out << std::hex << "0x" << naluInfo.m_info.m_position << std::dec << ": ";
  out << ConvToString::NALUnitType(naluInfo.m_pNALUnit -> m_nalUnitType) << std::endl;
}

void HEVCInfoWriter::writeVPS(std::shared_ptr<HEVC::VPS> pVPS, std::ostream &out)
{
  out << "\t" << "vps_video_parameter_set_id = " << (int) pVPS -> vps_video_parameter_set_id << std::endl;
  out << "\t" << "vps_max_layers_minus1 = " << (int) pVPS -> vps_max_layers_minus1 << std::endl;
  out << "\t" << "vps_max_sub_layers_minus1 = " << (int) pVPS -> vps_max_sub_layers_minus1 << std::endl;
  out << "\t" << "vps_temporal_id_nesting_flag = " << (int) pVPS -> vps_temporal_id_nesting_flag << std::endl;

  out << "\t" << "profile_tier_level:" << std::endl;
  writeProfileTierLevel(pVPS -> profile_tier_level, out, "\t\t");

  out << "\t" << "vps_sub_layer_ordering_info_present_flag = " << (int) pVPS -> vps_sub_layer_ordering_info_present_flag << std::endl;

  out << "\t" << "for( i = ( vps_sub_layer_ordering_info_present_flag ? 0 : vps_max_sub_layers_minus1 ); i <= vps_max_sub_layers_minus1; i++ )" << std::endl;

  for(std::size_t i = (pVPS -> vps_sub_layer_ordering_info_present_flag ? 0 : pVPS -> vps_max_sub_layers_minus1); i <= pVPS -> vps_max_sub_layers_minus1; i++)
  {
    out << "\t\t" << "vps_max_dec_pic_buffering_minus1[" << i << "] = " << (int) pVPS -> vps_max_dec_pic_buffering_minus1[i] << std::endl;
    out << "\t\t" << "vps_max_num_reorder_pics[" << i << "] = " << (int) pVPS -> vps_max_num_reorder_pics[i]<< std::endl;
    out << "\t\t" << "vps_max_latency_increase_plus1[" << i << "] = " << (int) pVPS -> vps_max_latency_increase_plus1[i]<< std::endl;
  }

  out << "\t" << "vps_max_layer_id = " << (int) pVPS -> vps_max_layer_id << std::endl;
  out << "\t" << "vps_num_layer_sets_minus1 = " << (int) pVPS -> vps_num_layer_sets_minus1 << std::endl;

  if(pVPS -> vps_num_layer_sets_minus1 == 0)
  {
      out << "\t\t" << "layer_id_included_flag = { }" << std::endl;
  }
  else
  {
    for(std::size_t i=0; i<=pVPS -> vps_num_layer_sets_minus1; i++)
    {
      out << "\t\t" << "layer_id_included_flag[" << i << "] = { ";
      for(std::size_t j=0; j<=pVPS -> vps_max_layer_id; j++)
        out << pVPS -> layer_id_included_flag[i][j] << ",  ";
      out << pVPS -> layer_id_included_flag[i][pVPS -> vps_max_layer_id - 1] << " } " << std::endl;
    }
  }


  out << "\t" << "vps_timing_info_present_flag = " << (int) pVPS -> vps_timing_info_present_flag << std::endl;


  if(pVPS -> vps_timing_info_present_flag)
  {
    out << "\t" << "if( vps_timing_info_present_flag )" << std::endl;

    out << "\t\t" << "vps_num_units_in_tick = " << (int) pVPS -> vps_num_units_in_tick << std::endl;
    out << "\t\t" << "vps_time_scale = " << (int) pVPS -> vps_time_scale << std::endl;
    out << "\t\t" << "vps_poc_proportional_to_timing_flag = " << (int) pVPS -> vps_poc_proportional_to_timing_flag << std::endl;

    if(pVPS -> vps_poc_proportional_to_timing_flag)
    {
      out << "\t\t" << "if( vps_poc_proportional_to_timing_flag )" << std::endl;
      out << "\t\t\t" << "vps_num_ticks_poc_diff_one_minus1 = " << (int) pVPS -> vps_num_ticks_poc_diff_one_minus1 << std::endl;
    }

    out << "\t\t" << "vps_num_hrd_parameters = " << (int) pVPS -> vps_num_hrd_parameters << std::endl;

    out << "\t\t" << "for( i = 0; i < vps_num_hrd_parameters; i++ )" << std::endl;

    for(std::size_t i=0; i<pVPS -> vps_num_hrd_parameters; i++)
    {
      out << "\t\t\t" << "hrd_layer_set_idx[" << i << "] = " << (int) pVPS -> hrd_layer_set_idx[i] << std::endl;
      if(i > 0)
      {
        out << "\t\t\t" << "if( i > 0 )" << std::endl;
        out << "\t\t\t\t" << "cprms_present_flag[" << i << "] = " << (int) pVPS -> cprms_present_flag[i] << std::endl;
      }
      out << "\t\t\t" << "hrd_parameters[" << i << "](" << pVPS -> cprms_present_flag[i] << ", " << pVPS -> vps_max_sub_layers_minus1 << "): " << std::endl;
      writeHrdParameters(pVPS -> hrd_parameters[i], pVPS -> cprms_present_flag[i], out, std::string("\t\t\t"));
    }
  }

  out << "\t" << "vps_extension_flag = " << (int) pVPS -> vps_extension_flag << std::endl;
}


void HEVCInfoWriter::writeSPS(std::shared_ptr<HEVC::SPS> pSPS, std::ostream &out)
{
  out << "\t" << "sps_video_parameter_set_id = " << (int) pSPS -> sps_video_parameter_set_id << std::endl;
  out << "\t" << "sps_max_sub_layers_minus1 = " << (int) pSPS -> sps_max_sub_layers_minus1 << std::endl;
  out << "\t" << "sps_temporal_id_nesting_flag = " << (int) pSPS -> sps_temporal_id_nesting_flag << std::endl;


  out << "\t" << "profile_tier_level:" << std::endl;
  writeProfileTierLevel(pSPS -> profile_tier_level, out, "\t\t");

  out << "\t" << "sps_seq_parameter_set_id = " << pSPS -> sps_seq_parameter_set_id << std::endl;
  out << "\t" << "chroma_format_idc = " << pSPS -> chroma_format_idc << std::endl;
  if(pSPS -> chroma_format_idc == 3)
  {
    out << "\t" << "if( chroma_format_idc == 3 )" << std::endl;
    out << "\t\t" << "separate_colour_plane_flag = " << (int) pSPS -> separate_colour_plane_flag << std::endl;
  }

  out << "\t" << "pic_width_in_luma_samples = " << pSPS -> pic_width_in_luma_samples << std::endl;
  out << "\t" << "pic_height_in_luma_samples = " << pSPS -> pic_height_in_luma_samples << std::endl;
  out << "\t" << "conformance_window_flag = " << (int) pSPS -> conformance_window_flag << std::endl;
  if(pSPS -> conformance_window_flag)
  {
    out << "\t" << "if( conformance_window_flag )" << std::endl;
    out << "\t\t" << "conf_win_left_offset = " << pSPS -> conf_win_left_offset << std::endl;
    out << "\t\t" << "conf_win_right_offset = " << pSPS -> conf_win_right_offset << std::endl;
    out << "\t\t" << "conf_win_top_offset = " << pSPS -> conf_win_top_offset << std::endl;
    out << "\t\t" << "conf_win_bottom_offset = " << pSPS -> conf_win_bottom_offset << std::endl;
  }

  out << "\t" << "bit_depth_luma_minus8 = " << pSPS -> bit_depth_luma_minus8 << std::endl;
  out << "\t" << "bit_depth_chroma_minus8 = " << pSPS -> bit_depth_chroma_minus8 << std::endl;
  out << "\t" << "log2_max_pic_order_cnt_lsb_minus4 = " << pSPS -> log2_max_pic_order_cnt_lsb_minus4 << std::endl;
  out << "\t" << "sps_sub_layer_ordering_info_present_flag = " << (int) pSPS -> sps_sub_layer_ordering_info_present_flag << std::endl;

  out << "\t" << "for( i = ( sps_sub_layer_ordering_info_present_flag ? 0 : sps_max_sub_layers_minus1 );i <= sps_max_sub_layers_minus1; i++ )" << std::endl;
  for(std::size_t i = (pSPS -> sps_sub_layer_ordering_info_present_flag ? 0 : pSPS -> sps_max_sub_layers_minus1); i <= pSPS -> sps_max_sub_layers_minus1; i++ )
  {
    out << "\t\t" << "sps_max_dec_pic_buffering_minus1[" << i << "] = " << pSPS -> sps_max_dec_pic_buffering_minus1[i] << std::endl;
    out << "\t\t" << "sps_max_num_reorder_pics[" << i << "] = " << pSPS -> sps_max_num_reorder_pics[i] << std::endl;
    out << "\t\t" << "sps_max_latency_increase_plus1[" << i << "] = " << pSPS -> sps_max_latency_increase_plus1[i] << std::endl;
  }


  out << "\tlog2_min_luma_coding_block_size_minus3" << " = " << pSPS -> log2_min_luma_coding_block_size_minus3 << std::endl;
  out << "\t" << "log2_diff_max_min_luma_coding_block_size = " << pSPS -> log2_diff_max_min_luma_coding_block_size << std::endl;
  out << "\t" << "log2_min_transform_block_size_minus2 = " << pSPS -> log2_min_transform_block_size_minus2 << std::endl;
  out << "\t" << "log2_diff_max_min_transform_block_size = " << pSPS -> log2_diff_max_min_transform_block_size << std::endl;
  out << "\t" << "max_transform_hierarchy_depth_inter = " << pSPS -> max_transform_hierarchy_depth_inter << std::endl;
  out << "\t" << "max_transform_hierarchy_depth_intra = " << pSPS -> max_transform_hierarchy_depth_intra << std::endl;
  out << "\t" << "scaling_list_enabled_flag = " << (int) pSPS -> scaling_list_enabled_flag << std::endl;

  if(pSPS -> scaling_list_enabled_flag)
  {
    out << "\t" << "if( scaling_list_enabled_flag )" << std::endl;
    out << "\t\t" << "sps_scaling_list_data_present_flag = " << (int) pSPS -> sps_scaling_list_data_present_flag << std::endl;
    //scaling_list_data( )
  }
  out << "\t" << "amp_enabled_flag = " << (int) pSPS -> amp_enabled_flag << std::endl;
  out << "\t" << "sample_adaptive_offset_enabled_flag = " << (int) pSPS -> sample_adaptive_offset_enabled_flag << std::endl;
  out << "\t" << "pcm_enabled_flag = " << (int) pSPS -> pcm_enabled_flag << std::endl;

  if(pSPS -> pcm_enabled_flag)
  {
    out << "\t" << "if( pcm_enabled_flag )" << std::endl;

    out << "\t\t" << "pcm_sample_bit_depth_luma_minus1 = " << pSPS -> pcm_sample_bit_depth_luma_minus1 << std::endl;
    out << "\t\t" << "pcm_sample_bit_depth_chroma_minus1 = " << pSPS -> pcm_sample_bit_depth_chroma_minus1 << std::endl;
    out << "\t\t" << "log2_min_pcm_luma_coding_block_size_minus3 = " << pSPS -> log2_min_pcm_luma_coding_block_size_minus3 << std::endl;
    out << "\t\t" << "log2_diff_max_min_pcm_luma_coding_block_size = " << pSPS -> log2_diff_max_min_pcm_luma_coding_block_size << std::endl;
    out << "\t\t" << "pcm_loop_filter_disabled_flag = " << pSPS -> pcm_loop_filter_disabled_flag << std::endl;

  }

  out << "\t" << "num_short_term_ref_pic_sets = " << pSPS -> num_short_term_ref_pic_sets << std::endl;

  out << "\t" << "for( i = 0; i < num_short_term_ref_pic_sets; i++)" << std::endl;
  for(std::size_t i=0; i<pSPS -> num_short_term_ref_pic_sets; i++)
  {
    out << "\t" << "short_term_ref_pic_set[" << i << "] (" << i << "):" << std::endl;
    writeShortTermRefPicSet(i, pSPS -> num_short_term_ref_pic_sets, pSPS -> short_term_ref_pic_set, out, "\t\t");
  }

  out << "\t" << "long_term_ref_pics_present_flag = " << (int) pSPS -> long_term_ref_pics_present_flag << std::endl;
  if(pSPS -> long_term_ref_pics_present_flag)
  {
    out << "\t" << "if( long_term_ref_pics_present_flag )" << std::endl;

    out << "\t\t" << "num_long_term_ref_pics_sps = " << (int) pSPS -> num_long_term_ref_pics_sps << std::endl;

    out << "\t" << "for( i = 0; i < num_long_term_ref_pics_sps; i++ )" << std::endl;

    for(std::size_t i=0; i<pSPS -> num_long_term_ref_pics_sps; i++)
    {
      out << "\t\t" << "lt_ref_pic_poc_lsb_sps[" << i << "] = " << (int) pSPS -> lt_ref_pic_poc_lsb_sps[i] << std::endl;
      out << "\t\t" << "used_by_curr_pic_lt_sps_flag[" << i << "] = " << (int) pSPS -> used_by_curr_pic_lt_sps_flag[i] << std::endl;
    }
  }

  out << "\t" << "sps_temporal_mvp_enabled_flag = " << (int) pSPS -> sps_temporal_mvp_enabled_flag << std::endl;
  out << "\t" << "strong_intra_smoothing_enabled_flag = " << (int) pSPS -> strong_intra_smoothing_enabled_flag << std::endl;
  out << "\t" << "vui_parameters_present_flag = " << (int) pSPS -> vui_parameters_present_flag << std::endl;

  if(pSPS -> vui_parameters_present_flag)
  {
    out << "\t" << "if( vui_parameters_present_flag )" << std::endl;
    out << "\t\t" << "vui_parameters(" << (int) pSPS -> sps_max_sub_layers_minus1 << "):" << std::endl;
    writeVuiParameters(pSPS -> vui_parameters, pSPS ->  sps_max_sub_layers_minus1, out, "\t\t\t");
  }

  out << "\t" << "sps_extension_flag = " << (int) pSPS -> sps_extension_flag << std::endl;
}


void HEVCInfoWriter::writePPS(std::shared_ptr<HEVC::PPS> pPPS, std::ostream &out)
{
  out << "\t" << "pps_pic_parameter_set_id = " << (int) pPPS -> pps_pic_parameter_set_id << std::endl;
  out << "\t" << "pps_seq_parameter_set_id = " << (int) pPPS -> pps_seq_parameter_set_id << std::endl;
  out << "\t" << "dependent_slice_segments_enabled_flag = " << (int) pPPS -> dependent_slice_segments_enabled_flag << std::endl;
  out << "\t" << "output_flag_present_flag = " << (int) pPPS -> output_flag_present_flag << std::endl;
  out << "\t" << "num_extra_slice_header_bits = " << (int) pPPS -> num_extra_slice_header_bits << std::endl;
  out << "\t" << "sign_data_hiding_flag = " << (int) pPPS -> sign_data_hiding_flag << std::endl;
  out << "\t" << "cabac_init_present_flag = " << (int) pPPS -> cabac_init_present_flag << std::endl;
  out << "\t" << "num_ref_idx_l0_default_active_minus1 = " << pPPS -> num_ref_idx_l0_default_active_minus1 << std::endl;
  out << "\t" << "num_ref_idx_l1_default_active_minus1 = " << pPPS -> num_ref_idx_l1_default_active_minus1 << std::endl;
  out << "\t" << "init_qp_minus26 = " << pPPS -> init_qp_minus26 << std::endl;
  out << "\t" << "constrained_intra_pred_flag = " << (int) pPPS -> constrained_intra_pred_flag << std::endl;
  out << "\t" << "transform_skip_enabled_flag = " << (int) pPPS -> transform_skip_enabled_flag << std::endl;
  out << "\t" << "cu_qp_delta_enabled_flag = " << (int) pPPS -> cu_qp_delta_enabled_flag << std::endl;
  if(pPPS -> cu_qp_delta_enabled_flag)
  {
    out << "\t" << "if( cu_qp_delta_enabled_flag )" << std::endl;
    out << "\t\t" << "diff_cu_qp_delta_depth = " << pPPS -> diff_cu_qp_delta_depth << std::endl;
  }

  out << "\t" << "pps_cb_qp_offset = " << pPPS -> pps_cb_qp_offset << std::endl;
  out << "\t" << "pps_cr_qp_offset = " << pPPS -> pps_cr_qp_offset << std::endl;
  out << "\t" << "pps_slice_chroma_qp_offsets_present_flag = " << (int) pPPS -> pps_slice_chroma_qp_offsets_present_flag << std::endl;
  out << "\t" << "weighted_pred_flag = " << (int) pPPS -> weighted_pred_flag << std::endl;
  out << "\t" << "weighted_bipred_flag = " << (int) pPPS -> weighted_bipred_flag << std::endl;
  out << "\t" << "transquant_bypass_enabled_flag = " << (int) pPPS -> transquant_bypass_enabled_flag << std::endl;
  out << "\t" << "tiles_enabled_flag = " << (int) pPPS -> tiles_enabled_flag << std::endl;
  out << "\t" << "entropy_coding_sync_enabled_flag = " << (int) pPPS -> entropy_coding_sync_enabled_flag << std::endl;

  if(pPPS -> tiles_enabled_flag)
  {
    out << "\t" << "if( tiles_enabled_flag )" << std::endl;

    out << "\t\t" << "num_tile_columns_minus1 = " << (int) pPPS -> num_tile_columns_minus1 << std::endl;
    out << "\t\t" << "num_tile_rows_minus1 = " << (int) pPPS -> num_tile_rows_minus1 << std::endl;
    out << "\t\t" << "uniform_spacing_flag = " << (int) pPPS -> uniform_spacing_flag << std::endl;

    if(!pPPS -> uniform_spacing_flag)
    {
      out << "\t" << "if( !uniform_spacing_flag )" << std::endl;

      out << "\t\t\t" << "column_width_minus1 = { " << std::endl;
      for(std::size_t i=0; i<pPPS -> num_tile_columns_minus1 - 1; i++)
        out << (int)  pPPS -> column_width_minus1[i] << ", ";
      out << (int) pPPS -> column_width_minus1[pPPS -> num_tile_columns_minus1 - 1] << " }" << std::endl;

      out << "\t\t\t" << "num_tile_rows_minus1 = { " << std::endl;
      for(std::size_t i=0; i<pPPS -> num_tile_rows_minus1 - 1; i++)
        out << (int)  pPPS -> row_height_minus1[i] << ", ";
      out << (int) pPPS -> row_height_minus1[pPPS -> num_tile_rows_minus1 - 1] << " }" << std::endl;
    }

    out << "\t\t" << "uniform_spacing_flag = " << (int) pPPS -> uniform_spacing_flag << std::endl;
  }

  out << "\t" << "pps_loop_filter_across_slices_enabled_flag = " << (int) pPPS -> pps_loop_filter_across_slices_enabled_flag << std::endl;
  out << "\t" << "deblocking_filter_control_present_flag = " << (int) pPPS -> deblocking_filter_control_present_flag << std::endl;

  if(pPPS -> deblocking_filter_control_present_flag)
  {
    out << "\t" << "if( deblocking_filter_control_present_flag )" << std::endl;
    out << "\t\t" << "deblocking_filter_override_enabled_flag = " << (int) pPPS -> deblocking_filter_override_enabled_flag << std::endl;
    out << "\t\t" << "pps_deblocking_filter_disabled_flag = " << (int) pPPS -> pps_deblocking_filter_disabled_flag << std::endl;

    if(!pPPS -> pps_deblocking_filter_disabled_flag)
    {
      out << "\t\t" << "if( !pps_deblocking_filter_disabled_flag )" << std::endl;
      out << "\t\t\t" << "pps_beta_offset_div2 = " << (int) pPPS -> pps_beta_offset_div2 << std::endl;
      out << "\t\t\t" << "pps_tc_offset_div2 = " << (int) pPPS -> pps_tc_offset_div2 << std::endl;
    }
  }
}



void HEVCInfoWriter::writeSlice(std::shared_ptr<HEVC::Slice> pSlice, std::ostream &out)
{
  std::shared_ptr<PPS> pPPS = m_ppsMap[pSlice -> slice_pic_parameter_set_id];
  if(!pPPS)
    return;
  int32_t spsId = pPPS -> pps_seq_parameter_set_id;


  out << "\t" << "first_slice_segment_in_pic_flag = " << (int) pSlice -> first_slice_segment_in_pic_flag << std::endl;
  if(pSlice -> m_nalUnitType >= HEVC::NAL_BLA_W_LP && pSlice -> m_nalUnitType <= HEVC::NAL_IRAP_VCL23)
  {
    out << "\t" << "if( nal_unit_type >= BLA_W_LP && nal_unit_type <= RSV_IRAP_VCL23 )" << std::endl;
    out << "\t\t" << "no_output_of_prior_pics_flag = " << (int) pSlice -> no_output_of_prior_pics_flag << std::endl;
  }

  out << "\t" << "slice_pic_parameter_set_id = " << pSlice -> slice_pic_parameter_set_id << std::endl;


  if(!pSlice -> first_slice_segment_in_pic_flag)
  {
    out << "\t" << "if( !first_slice_segment_in_pic_flag )" << std::endl;

    if(pPPS -> dependent_slice_segments_enabled_flag)
    {
      out << "\t\t" << "if( dependent_slice_segments_enabled_flag )" << std::endl;
      out << "\t\t\t" << "dependent_slice_segment_flag = " << (int)pSlice -> dependent_slice_segment_flag << std::endl;
    }

    out << "\t\t" << "slice_segment_address = " << pSlice -> slice_segment_address << std::endl;
  }


  if(!pSlice -> dependent_slice_segment_flag)
  {
    out << "\t" << "if( !dependent_slice_segment_flag )" << std::endl;
    long num_extra_slice_header_bits = pPPS -> num_extra_slice_header_bits;
    out << "\t" << "slice_reserved_undetermined_flag = { ";

    if(num_extra_slice_header_bits > 0)
    {
      out << "\t\t" << "" << std::endl;
      for(long i=0; i<num_extra_slice_header_bits - 1; i++)
        out << pSlice -> slice_reserved_undetermined_flag[i] << ", ";
      out << pSlice -> slice_reserved_undetermined_flag[num_extra_slice_header_bits - 1] << std::endl;
    }
    out << " }" << std::endl;

    out << "\t" << "slice_type = " << pSlice -> slice_type << std::endl;

    if(pPPS -> output_flag_present_flag)
    {
      out << "\t" << "if( output_flag_present_flag )" << std::endl;
      out << "\t\t" << "pic_output_flag = " << (int) pSlice -> pic_output_flag << std::endl;
    }

    if(m_spsMap[spsId] -> separate_colour_plane_flag)
    {
      out << "\t" << "if( separate_colour_plane_flag = = 1 )" << std::endl;
      out << "\t\t" << "colour_plane_id = " << (int) pSlice -> colour_plane_id << std::endl;
    }

    bool IdrPicFlag = pSlice -> m_nalUnitType == NAL_IDR_W_RADL || pSlice -> m_nalUnitType == NAL_IDR_N_LP;
    if(!IdrPicFlag)
    {
      out << "\t" << "if( nal_unit_type != IDR_W_RADL && nal_unit_type != IDR_N_LP )" << std::endl;
      out << "\t\t" << "slice_pic_order_cnt_lsb = " << pSlice -> slice_pic_order_cnt_lsb << std::endl;
      out << "\t\t" << "short_term_ref_pic_set_sps_flag = " << (int) pSlice -> short_term_ref_pic_set_sps_flag << std::endl;

      if(!pSlice -> short_term_ref_pic_set_sps_flag)
      {
        out << "\t\t" << "if( !short_term_ref_pic_set_sps_flag )" << std::endl;
        //short_term_ref_pic_set
      }
      else if(pSlice -> short_term_ref_pic_set_sps_flag > 1)
      {
        out << "\t\t" << "if(short_term_ref_pic_set_sps_flag && num_short_term_ref_pic_sets > 1 )" << std::endl;
        std::size_t numBits = HEVC::log2(m_spsMap[spsId] -> num_short_term_ref_pic_sets);
        if(numBits > 0)
          out << "\t\t\t" << "short_term_ref_pic_set_idx = " << (int) pSlice -> short_term_ref_pic_set_idx << std::endl;
      }
    }
  }
}

void HEVCInfoWriter::writeAUD(std::shared_ptr<HEVC::AUD> pAUD, std::ostream &out)
{
  out << "\t" << "pic_type = " << (int) pAUD -> pic_type << std::endl;
}

void HEVCInfoWriter::writeSEI(std::shared_ptr<HEVC::SEI> pSEI, std::ostream &out)
{
  std::size_t payloadType = 0;
  std::size_t payloadSize = 0;
  for(std::size_t i=0; i< pSEI -> sei_message.size(); i++)
  {
    out << "\t" << "sei_message(" << (int) i << ")\n";
    if(pSEI -> sei_message[i].num_payload_type_ff_bytes)
    {
      out << "\t\t" << "while( next_bits( 8 ) == 0xFF )" << std::endl;
      for(std::size_t i=0; i<pSEI -> sei_message[i].num_payload_type_ff_bytes; i++)
      {
        out << "\t\t\t" << "0xFF" << std::endl;
        payloadType += 255;
      }
    }

    out << "\t\t" << "last_payload_type_byte = " << (int) pSEI -> sei_message[i].last_payload_type_byte << std::endl;
    payloadType += pSEI -> sei_message[i].last_payload_type_byte;

    if(pSEI -> sei_message[i].num_payload_size_ff_bytes)
    {
      out << "\t\t" << "while( next_bits( 8 ) == 0xFF )" << std::endl;
      for(std::size_t j=0; j<pSEI -> sei_message[i].num_payload_size_ff_bytes; j++)
      {
        out << "\t\t\t" << "0xFF" << std::endl;
        payloadSize += 255;
      }
    }

    out << "\t\t" << "last_payload_size_byte = " << (int) pSEI -> sei_message[i].last_payload_size_byte << std::endl;
    payloadSize += pSEI -> sei_message[i].last_payload_size_byte;

    out << "\t\t" << "sei_payload( " << (int) payloadType << ", " << (int) payloadSize << " )" << std::endl;
  }

}


void HEVCInfoWriter::writeProfileTierLevel(const ProfileTierLevel &ptl, std::ostream &out, const std::string &prefix)
{
  out << prefix << "general_profile_space = " << (int) ptl.general_profile_space << std::endl;
  out << prefix << "general_tier_flag = " << (int) ptl.general_tier_flag << std::endl;
  out << prefix << "general_profile_idc = " << (int) ptl.general_profile_idc << std::endl;

  out << prefix << "general_profile_compatibility_flag[i] = { ";
  for(std::size_t i=0; i<31; i++)
    out << (int) ptl.general_profile_compatibility_flag[i] << ", ";
  out << (int) ptl.general_profile_compatibility_flag[31] << " }" << std::endl;;

  out << prefix << "general_progressive_source_flag = " << (int) ptl.general_progressive_source_flag << std::endl;
  out << prefix << "general_interlaced_source_flag = " << (int) ptl.general_interlaced_source_flag << std::endl;
  out << prefix << "general_non_packed_constraint_flag = " << (int) ptl.general_non_packed_constraint_flag << std::endl;
  out << prefix << "general_frame_only_constraint_flag = " << (int) ptl.general_frame_only_constraint_flag << std::endl;
  out << prefix << "general_level_idc = " << (int) ptl.general_level_idc << std::endl;

  std::size_t maxNumSubLayersMinus1 = ptl.sub_layer_profile_present_flag.size();

  if(maxNumSubLayersMinus1 == 0)
  {
    out << prefix << "sub_layer_profile_present_flag = {}" << std::endl;
    out << prefix << "sub_layer_level_present_flag = {}" << std::endl;
  }
  else
  {
    out << prefix << "sub_layer_profile_present_flag = { ";
    for(std::size_t i=0; i<maxNumSubLayersMinus1 - 1; i++)
      out << (int) ptl.sub_layer_profile_present_flag[i] << ", ";
    out << (int) ptl.sub_layer_profile_present_flag[maxNumSubLayersMinus1 - 1] << " }" << std::endl;

    out << prefix << "sub_layer_level_present_flag = { ";
    for(std::size_t i=0; i<maxNumSubLayersMinus1 - 1; i++)
      out << (int) ptl.sub_layer_level_present_flag[i] << ", ";
    out << (int) ptl.sub_layer_level_present_flag[maxNumSubLayersMinus1 - 1] << " }" << std::endl;
  }

  out << prefix << "for( i = 0; i < maxNumSubLayersMinus1; i++ )" << std::endl;
  for(std::size_t i=0; i<maxNumSubLayersMinus1; i++)
  {
    if(ptl.sub_layer_profile_present_flag[i])
    {
      out << prefix << "\t" << "if( sub_layer_profile_present_flag[ i ] )" << std::endl;

      out << prefix << "\t\t" << "sub_layer_profile_space[" << i << "] = " << (int) ptl.sub_layer_profile_space[i] << std::endl;
      out << prefix << "\t\t" << "sub_layer_tier_flag[" << i << "] = " << (int) ptl.sub_layer_tier_flag[i] << std::endl;
      out << prefix << "\t\t" << "sub_layer_profile_idc[" << i << "] = " << (int) ptl.sub_layer_profile_idc[i] << std::endl;
      out << prefix << "\t\t" << "sub_layer_profile_compatibility_flag[" << i << "] = {";
      for(std::size_t j=0; j<31; j++)
        out << (int) ptl.sub_layer_profile_compatibility_flag[i][j] << ", ";
      out << (int) ptl.sub_layer_profile_compatibility_flag[i][31] << " }" << std::endl;

      out << prefix << "\t\t" << "sub_layer_progressive_source_flag[" << i << "] = " << (int) ptl.sub_layer_progressive_source_flag[i] << std::endl;
      out << prefix << "\t\t" << "sub_layer_interlaced_source_flag[" << i << "] = " << (int) ptl.sub_layer_interlaced_source_flag[i] << std::endl;
      out << prefix << "\t\t" << "sub_layer_non_packed_constraint_flag[" << i << "] = " << (int) ptl.sub_layer_non_packed_constraint_flag[i] << std::endl;
      out << prefix << "\t\t" << "sub_layer_frame_only_constraint_flag[" << i << "] = " << (int) ptl.sub_layer_frame_only_constraint_flag[i] << std::endl;
    }

    if(ptl.sub_layer_level_present_flag[i])
    {
      out << prefix << "if( sub_layer_level_present_flag[ i ] )" << std::endl;
      out << prefix << "\t" << "sub_layer_level_idc[" << i << "] = " << (int) ptl.sub_layer_level_idc[i] << std::endl;
    }
  }
}


void HEVCInfoWriter::writeHrdParameters(const HEVC::HrdParameters &hrd, uint8_t commonInfPresentFlag, std::ostream &out, const std::string &prefix)
{
  if(commonInfPresentFlag)
  {
    out << prefix << "if( commonInfPresentFlag )" << std::endl;
    out << prefix << "\t" << "nal_hrd_parameters_present_flag = " << (int) hrd.nal_hrd_parameters_present_flag << std::endl;
    out << prefix << "\t" << "vcl_hrd_parameters_present_flag = " << (int) hrd.vcl_hrd_parameters_present_flag << std::endl;

    if(hrd.nal_hrd_parameters_present_flag || hrd.vcl_hrd_parameters_present_flag)
    {
      out << prefix << "" << std::endl;
      out << prefix << "\t" << "if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag) " << std::endl;

      out << prefix << "\t\t" << "sub_pic_hrd_params_present_flag = " << (int) hrd.sub_pic_hrd_params_present_flag << std::endl;
      if(hrd.sub_pic_hrd_params_present_flag)
      {
        out << prefix << "\t\t" << "if( sub_pic_hrd_params_present_flag )" << std::endl;
        out << prefix << "\t\t\t" << "tick_divisor_minus2 = " << (int) hrd.tick_divisor_minus2 << std::endl;
        out << prefix << "\t\t\t" << "du_cpb_removal_delay_increment_length_minus1 = " << (int) hrd.du_cpb_removal_delay_increment_length_minus1 << std::endl;
        out << prefix << "\t\t\t" << "sub_pic_cpb_params_in_pic_timing_sei_flag = " << (int) hrd.sub_pic_cpb_params_in_pic_timing_sei_flag << std::endl;
        out << prefix << "\t\t\t" << "dpb_output_delay_du_length_minus1 = " << (int) hrd.dpb_output_delay_du_length_minus1 << std::endl;
      }

      out << prefix << "\t\t" << "bit_rate_scale = " << (int) hrd.bit_rate_scale << std::endl;
      out << prefix << "\t\t" << "cpb_size_scale = " << (int) hrd.cpb_size_scale << std::endl;

      if(hrd.sub_pic_hrd_params_present_flag)
      {
        out << prefix << "if (sub_pic_hrd_params_present_flag )" << std::endl;
        out << prefix << "\t\t" << "cpb_size_du_scale = " << (int) hrd.cpb_size_du_scale << std::endl;
      }

      out << prefix << "\t\t" << "initial_cpb_removal_delay_length_minus1 = " << (int) hrd.initial_cpb_removal_delay_length_minus1 << std::endl;
      out << prefix << "\t\t" << "au_cpb_removal_delay_length_minus1 = " << (int) hrd.au_cpb_removal_delay_length_minus1 << std::endl;
      out << prefix << "\t\t" << "dpb_output_delay_length_minus1 = " << (int) hrd.dpb_output_delay_length_minus1 << std::endl;
    }
  }

  out << prefix << "for( i = 0; i <= maxNumSubLayersMinus1; i++ )" << std::endl;
  for(std::size_t i = 0; i < hrd.fixed_pic_rate_general_flag.size(); i++ )
  {
    out << prefix << "\t\t" << "fixed_pic_rate_general_flag[" << i << "] = " << (int) hrd.fixed_pic_rate_general_flag[i] << std::endl;
    if(!hrd.fixed_pic_rate_general_flag[i])
    {
      out << prefix << "\t\t" <<  "if( !fixed_pic_rate_general_flag[ i ] )" << std::endl;
      out << prefix << "\t\t\t" << "fixed_pic_rate_within_cvs_flag[" << i << "] = " << (int) hrd.fixed_pic_rate_within_cvs_flag[i] << std::endl;
    }

    if(hrd.fixed_pic_rate_within_cvs_flag[i])
    {
      out << prefix << "\t\t" << "if( fixed_pic_rate_within_cvs_flag[ i ] )" << std::endl;
      out << prefix << "\t\t\t" << "elemental_duration_in_tc_minus1[" << i << "] = " << (int) hrd.elemental_duration_in_tc_minus1[i] << std::endl;
    }
    else
    {
      out << prefix << "\t\t" <<  "if( !fixed_pic_rate_within_cvs_flag[ i ] )" << std::endl;
      out << prefix << "\t\t\t" << "low_delay_hrd_flag[" << i << "] = " << (int) hrd.low_delay_hrd_flag[i] << std::endl;
    }

    if(!hrd.low_delay_hrd_flag[i])
    {
      out << prefix << "\t\t" <<  "if( !low_delay_hrd_flag[ i ] )" << std::endl;
      out << prefix << "\t\t\t" << "cpb_cnt_minus1[" << i << "] = " << (int) hrd.cpb_cnt_minus1[i] << std::endl;
    }

    if(hrd.nal_hrd_parameters_present_flag)
    {
      out << prefix << "\t\t" <<  "if( nal_hrd_parameters_present_flag )" << std::endl;
      out << prefix << "\t" << "sub_layer_hrd_parameters(" << i << ")" << std::endl;
      writeSubLayerHrdParameters(hrd.nal_sub_layer_hrd_parameters[i], i,out, prefix + "\t");
    }
    if(hrd.vcl_hrd_parameters_present_flag)
    {
      out << prefix << "\t\t" <<  "if( vcl_hrd_parameters_present_flag )" << std::endl;
      out << prefix << "\t" << "sub_layer_hrd_parameters(" << i << ")" << std::endl;
      writeSubLayerHrdParameters(hrd.vcl_sub_layer_hrd_parameters[i], i, out, prefix + "\t");

    }
  }
}


void HEVCInfoWriter::writeSubLayerHrdParameters(const HEVC::SubLayerHrdParameters &slhrd, uint8_t sub_pic_hrd_params_present_flag, std::ostream &out, const std::string &prefix)
{

  out << prefix << "for( i = 0; i <= CpbCnt; i++ )" << std::endl;

  for(std::size_t i=0; i<slhrd.bit_rate_value_minus1.size(); i++)
  {
    out << prefix << "\t" << "bit_rate_value_minus1[" << i << "] = " << (int) slhrd.bit_rate_value_minus1[i] << std::endl;
    out << prefix << "\t" << "cpb_size_value_minus1[" << i << "] = " << (int) slhrd.cpb_size_value_minus1[i] << std::endl;

    if(sub_pic_hrd_params_present_flag)
    {
      out << prefix << "\t" << "if( sub_pic_hrd_params_present_flag )" << std::endl;
      out << prefix << "\t\t" << "cpb_size_du_value_minus1[" << i << "] = " << (int) slhrd.cpb_size_du_value_minus1[i] << std::endl;
      out << prefix << "\t\t" << "bit_rate_du_value_minus1[" << i << "] = " << (int) slhrd.bit_rate_du_value_minus1[i] << std::endl;
    }

    out << prefix << "\t" << "cbr_flag[" << i << "] = " << (int) slhrd.cbr_flag[i] << std::endl;
  }

}


void HEVCInfoWriter::writeShortTermRefPicSet(std::size_t stRpsIdx, std::size_t num_short_term_ref_pic_sets, const std::vector<ShortTermRefPicSet> &refPicSets, std::ostream &out, const std::string &prefix)
{
  ShortTermRefPicSet rpset = refPicSets[stRpsIdx];

  if(stRpsIdx)
  {
    out << prefix << "if( stRpsIdx != 0 )" << std::endl;
    out << prefix << "\t" << "inter_ref_pic_set_prediction_flag = " << (int) refPicSets[stRpsIdx].inter_ref_pic_set_prediction_flag << std::endl;
  }

  if(rpset.inter_ref_pic_set_prediction_flag)
  {
    out << prefix << "if( inter_ref_pic_set_prediction_flag )" << std::endl;

    if(stRpsIdx == num_short_term_ref_pic_sets)
    {
      out << prefix << "\t" << "if( stRpsIdx == num_short_term_ref_pic_sets )" << std::endl;
      out << prefix << "\t\t" << "delta_idx_minus1 = " << (int) refPicSets[stRpsIdx].delta_idx_minus1 << std::endl;
    }

    out << prefix << "\t" << "delta_rps_sign = " << (int) refPicSets[stRpsIdx].delta_rps_sign << std::endl;
    out << prefix << "\t" << "abs_delta_rps_minus1 = " << (int) refPicSets[stRpsIdx].abs_delta_rps_minus1 << std::endl;

    std::size_t RefRpsIdx = stRpsIdx - (rpset.delta_idx_minus1+1);
    std::size_t NumDeltaPocs = refPicSets[RefRpsIdx].num_negative_pics + refPicSets[RefRpsIdx].num_negative_pics;

    out << prefix << "for( j = 0; j <= NumDeltaPocs[ RefRpsIdx ]; j++ )" << std::endl;
    for(std::size_t i=0; i<=NumDeltaPocs; i++ )
    {
      out << prefix << "\t\t" << "used_by_curr_pic_flag [" << i << "] = " << (int) refPicSets[stRpsIdx].used_by_curr_pic_flag[i] << std::endl;
      if(!rpset.used_by_curr_pic_flag[i])
      {
        out << prefix << "\t\t" << "if( !used_by_curr_pic_flag[j] )" << std::endl;
        out << prefix << "\t\t\t" << "use_delta_flag [" << i << "] = " << (int) refPicSets[stRpsIdx].use_delta_flag[i] << std::endl;
      }
    }
  }
  else
  {
    out << prefix << "if( !inter_ref_pic_set_prediction_flag )" << std::endl;

    out << prefix << "\t" << "num_negative_pics = " << (int) refPicSets[stRpsIdx].num_negative_pics << std::endl;
    out << prefix << "\t" << "num_positive_pics = " << (int) refPicSets[stRpsIdx].num_positive_pics << std::endl;

    out << prefix << "\t" << "for( i = 0; i < num_negative_pics; i++ )" << std::endl;
    for(std::size_t i=0; i<rpset.num_negative_pics; i++)
    {
      out << prefix << "\t\t" << "delta_poc_s0_minus1[" << i <<  "] = " << (int) refPicSets[stRpsIdx].delta_poc_s0_minus1[i] << std::endl;
      out << prefix << "\t\t" << "used_by_curr_pic_s0_flag[" << i <<  "] = " << (int) refPicSets[stRpsIdx].used_by_curr_pic_s0_flag[i] << std::endl;
    }

    out << prefix << "for( i = 0; i < num_positive_pics; i++ )" << std::endl;
    for(std::size_t i=0; i<rpset.num_positive_pics; i++)
    {
      out << prefix << "\t\t" << "delta_poc_s1_minus1[" << i <<  "] = " << (int) refPicSets[stRpsIdx].delta_poc_s1_minus1[i] << std::endl;
      out << prefix << "\t\t" << "used_by_curr_pic_s1_flag[" << i <<  "] = " << (int) refPicSets[stRpsIdx].used_by_curr_pic_s1_flag[i] << std::endl;
    }

  }
}


void HEVCInfoWriter::writeVuiParameters(const HEVC::VuiParameters &vui, std::size_t sps_max_sub_layers_minus1, std::ostream &out, const std::string &prefix)
{
  out << prefix << "aspect_ratio_info_present_flag = " << (int) vui.aspect_ratio_info_present_flag << std::endl;

  if(vui.aspect_ratio_info_present_flag)
  {
    out << prefix << "if( aspect_ratio_info_present_flag )" << std::endl;
    out << prefix << "\t" << "aspect_ratio_idc = " << (int) vui.aspect_ratio_idc << std::endl;

    if(vui.aspect_ratio_idc == 255) //EXTENDED_SAR
    {
      out << prefix << "\t" <<  "if( aspect_ratio_idc = = EXTENDED_SAR )" << std::endl;
      out << prefix << "\t\t" << "sar_width = " << (int) vui.sar_width << std::endl;
      out << prefix << "\t\t" << "sar_height = " << (int) vui.sar_height << std::endl;
    }
  }

  out << prefix << "overscan_info_present_flag = " << (int) vui.overscan_info_present_flag<< std::endl;
  if(vui.overscan_info_present_flag)
  {
    out << prefix << "if( overscan_info_present_flag )" << std::endl;
    out << prefix << "\t" << "overscan_appropriate_flag = " << (int) vui.overscan_appropriate_flag << std::endl;
  }

  out << prefix << "video_signal_type_present_flag = " << (int) vui.video_signal_type_present_flag << std::endl;

  if(vui.video_signal_type_present_flag)
  {
    out << prefix << "if( video_signal_type_present_flag )" << std::endl;
    out << prefix << "\t" << "video_format = " << (int) vui.video_format << std::endl;
    out << prefix << "\t" << "video_full_range_flag = " << (int) vui.video_full_range_flag << std::endl;
    out << prefix << "\t" << "colour_description_present_flag = " << (int) vui.colour_description_present_flag << std::endl;

    if(vui.colour_description_present_flag)
    {
      out << prefix << "\t" <<  "if( colour_description_present_flag )" << std::endl;
      out << prefix << "\t\t" << "colour_primaries = " << (int) vui.colour_primaries << std::endl;
      out << prefix << "\t\t" << "transfer_characteristics = " << (int) vui.transfer_characteristics << std::endl;
      out << prefix << "\t\t" << "matrix_coeffs = " << (int) vui.matrix_coeffs << std::endl;
    }

  }

  out << prefix << "chroma_loc_info_present_flag = " << (int) vui.chroma_loc_info_present_flag << std::endl;

  if(vui.chroma_loc_info_present_flag)
  {
    out << prefix << "if( chroma_loc_info_present_flag )" << std::endl;
    out << prefix << "\t" << "chroma_sample_loc_type_top_field = " << (int) vui.chroma_sample_loc_type_top_field << std::endl;
    out << prefix << "\t" << "chroma_sample_loc_type_bottom_field = " << (int) vui.chroma_sample_loc_type_bottom_field << std::endl;
  }

  out << prefix << "neutral_chroma_indication_flag = " << (int) vui.neutral_chroma_indication_flag << std::endl;
  out << prefix << "field_seq_flag = " << (int) vui.field_seq_flag << std::endl;
  out << prefix << "frame_field_info_present_flag = " << (int) vui.frame_field_info_present_flag << std::endl;
  out << prefix << "default_display_window_flag = " << (int) vui.default_display_window_flag << std::endl;

  if(vui.default_display_window_flag)
  {
    out << prefix << "if( default_display_window_flag )" << std::endl;
    out << prefix << "\t" << "def_disp_win_left_offset = " << (int) vui.def_disp_win_left_offset << std::endl;
    out << prefix << "\t" << "def_disp_win_right_offset = " << (int) vui.def_disp_win_right_offset << std::endl;
    out << prefix << "\t" << "def_disp_win_top_offset = " << (int) vui.def_disp_win_top_offset << std::endl;
    out << prefix << "\t" << "def_disp_win_bottom_offset = " << (int) vui.def_disp_win_bottom_offset << std::endl;
  }

  out << prefix << "vui_timing_info_present_flag = " << (int) vui.vui_timing_info_present_flag << std::endl;

  if(vui.vui_timing_info_present_flag)
  {
    out << prefix << "if( vui_timing_info_present_flag )" << std::endl;
    out << prefix << "\t" << "vui_num_units_in_tick = " << (int) vui.vui_num_units_in_tick << std::endl;
    out << prefix << "\t" << "vui_time_scale = " << (int) vui.vui_time_scale << std::endl;
    out << prefix << "\t" << "vui_poc_proportional_to_timing_flag = " << (int) vui.vui_poc_proportional_to_timing_flag << std::endl;

    if(vui.vui_poc_proportional_to_timing_flag)
    {
      out << prefix << "\t" <<  "if( vui_poc_proportional_to_timing_flag )" << std::endl;
      out << prefix << "\t\t" << "vui_num_ticks_poc_diff_one_minus1 = " << (int) vui.vui_num_ticks_poc_diff_one_minus1 << std::endl;
    }

    out << prefix << "\t" << "vui_hrd_parameters_present_flag = " << (int) vui.vui_hrd_parameters_present_flag << std::endl;

    if(vui.vui_hrd_parameters_present_flag)
    {
      out << prefix << "\t" <<  "if( vui_hrd_parameters_present_flag )" << std::endl;
      out << prefix << "\t\t" << "hrd_parameters(" << 1 << ", " << sps_max_sub_layers_minus1 << "): " << std::endl;
      writeHrdParameters(vui.hrd_parameters, 1, out, prefix + "\t\t\t");
    }
  }

  out << prefix << "bitstream_restriction_flag = " << (int) vui.bitstream_restriction_flag << std::endl;

  if(vui.bitstream_restriction_flag)
  {
    out << prefix << "if( bitstream_restriction_flag )" << std::endl;
    out << prefix << "\t" << "tiles_fixed_structure_flag = " << (int) vui.tiles_fixed_structure_flag << std::endl;
    out << prefix << "\t" << "motion_vectors_over_pic_boundaries_flag = " << (int) vui.motion_vectors_over_pic_boundaries_flag << std::endl;
    out << prefix << "\t" << "restricted_ref_pic_lists_flag = " << (int) vui.restricted_ref_pic_lists_flag << std::endl;
    out << prefix << "\t" << "min_spatial_segmentation_idc = " << (int) vui.min_spatial_segmentation_idc << std::endl;
    out << prefix << "\t" << "max_bytes_per_pic_denom = " << (int) vui.max_bytes_per_pic_denom << std::endl;
    out << prefix << "\t" << "max_bits_per_min_cu_denom = " << (int) vui.max_bits_per_min_cu_denom << std::endl;
    out << prefix << "\t" << "log2_max_mv_length_horizontal = " << (int) vui.log2_max_mv_length_horizontal << std::endl;
    out << prefix << "\t" << "log2_max_mv_length_vertical = " << (int) vui.log2_max_mv_length_vertical << std::endl;
  }
}
