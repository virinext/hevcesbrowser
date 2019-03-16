#include "HEVCInfoAltWriter.h"

#include <HevcUtils.h>
#include <ConvToString.h>

#include <iostream>

using namespace HEVC;

void HEVCInfoAltWriter::writeNALHeader(const NALUInfo& naluInfo, std::ostream &out)
{
  out << "offset: " << std::hex << "0x" << naluInfo.m_info.m_position << std::dec << " ";
  out << "nal_unit_header { ";
  out << "type: " << ConvToString::NALUnitType(naluInfo.m_pNALUnit -> m_nalUnitType) << " ";
  out << "} ";
}

void HEVCInfoAltWriter::writeVPS(std::shared_ptr<HEVC::VPS> pVPS, std::ostream &out)
{
  out << "video_parameter_set_rbsp { ";
  out << "vps_video_parameter_set_id: " << (int) pVPS -> vps_video_parameter_set_id << " ";
  out << "vps_max_layers_minus1: " << (int) pVPS -> vps_max_layers_minus1 << " ";
  out << "vps_max_sub_layers_minus1: " << (int) pVPS -> vps_max_sub_layers_minus1 << " ";
  out << "vps_temporal_id_nesting_flag: " << (int) pVPS -> vps_temporal_id_nesting_flag << " ";

  out << "profile_tier_level { ";
  writeProfileTierLevel(pVPS -> profile_tier_level, out);
  out << "} ";

  out << "vps_sub_layer_ordering_info_present_flag: " << (int) pVPS -> vps_sub_layer_ordering_info_present_flag << " ";

  out << "vps_sub_layer_ordering_info { ";
  for(std::size_t i = (pVPS -> vps_sub_layer_ordering_info_present_flag ? 0 : pVPS -> vps_max_sub_layers_minus1); i <= pVPS -> vps_max_sub_layers_minus1; i++)
  {
    out << "vps_max_dec_pic_buffering_minus1[" << i << "]: " << (int) pVPS -> vps_max_dec_pic_buffering_minus1[i] << " ";
    out << "vps_max_num_reorder_pics[" << i << "]: " << (int) pVPS -> vps_max_num_reorder_pics[i] << " ";
    out << "vps_max_latency_increase_plus1[" << i << "]: " << (int) pVPS -> vps_max_latency_increase_plus1[i] << " ";
  }
  out << "} ";

  out << "vps_max_layer_id: " << (int) pVPS -> vps_max_layer_id << " ";
  out << "vps_num_layer_sets_minus1: " << (int) pVPS -> vps_num_layer_sets_minus1 << " ";

  if(pVPS -> vps_num_layer_sets_minus1 != 0)
  {
    for(std::size_t i=0; i<=pVPS -> vps_num_layer_sets_minus1; i++) {
      for(std::size_t j=0; j<=pVPS -> vps_max_layer_id; j++) {
        out << "layer_id_included_flag[" << i << "][" << j << "]: " << pVPS -> layer_id_included_flag[i][j] << "  ";
      }
    }
  }


  out << "vps_timing_info_present_flag: " << (int) pVPS -> vps_timing_info_present_flag << " ";


  out << "vps_timing_info { ";
  if(pVPS -> vps_timing_info_present_flag)
  {
    out << "vps_num_units_in_tick: " << (int) pVPS -> vps_num_units_in_tick << " ";
    out << "vps_time_scale: " << (int) pVPS -> vps_time_scale << " ";
    out << "vps_poc_proportional_to_timing_flag: " << (int) pVPS -> vps_poc_proportional_to_timing_flag << " ";

    if(pVPS -> vps_poc_proportional_to_timing_flag)
    {
      out << "vps_num_ticks_poc_diff_one_minus1: " << (int) pVPS -> vps_num_ticks_poc_diff_one_minus1 << " ";
    }

    out << "vps_num_hrd_parameters: " << (int) pVPS -> vps_num_hrd_parameters << " ";

    out << "for( i = 0; i < vps_num_hrd_parameters; i++ )";

    for(std::size_t i=0; i<pVPS -> vps_num_hrd_parameters; i++)
    {
      out << "hrd_layer_set_idx[" << i << "]: " << (int) pVPS -> hrd_layer_set_idx[i] << " ";
      if(i > 0)
      {
        out << "cprms_present_flag[" << i << "]: " << (int) pVPS -> cprms_present_flag[i] << " ";
      }
      out << "hrd_parameters[" << i << "](" << pVPS -> cprms_present_flag[i] << ", " << pVPS -> vps_max_sub_layers_minus1 << ") { ";
      writeHrdParameters(pVPS -> hrd_parameters[i], pVPS -> cprms_present_flag[i], out);
      out << "} ";
    }
  }
  out << "} ";

  out << "vps_extension_flag: " << (int) pVPS -> vps_extension_flag << " ";
  out << " }";
}


void HEVCInfoAltWriter::writeSPS(std::shared_ptr<HEVC::SPS> pSPS, std::ostream &out)
{
  out << "sps_video_parameter_set_id: " << (int) pSPS -> sps_video_parameter_set_id << " ";
  out << "sps_max_sub_layers_minus1: " << (int) pSPS -> sps_max_sub_layers_minus1 << " ";
  out << "sps_temporal_id_nesting_flag: " << (int) pSPS -> sps_temporal_id_nesting_flag << " ";


  out << "profile_tier_level { ";
  writeProfileTierLevel(pSPS -> profile_tier_level, out);
  out << "} ";

  out << "sps_seq_parameter_set_id: " << pSPS -> sps_seq_parameter_set_id << " ";
  out << "chroma_format_idc: " << pSPS -> chroma_format_idc << " ";
  if(pSPS -> chroma_format_idc == 3)
  {
    out << "separate_colour_plane_flag: " << (int) pSPS -> separate_colour_plane_flag << " ";
  }

  out << "pic_width_in_luma_samples: " << pSPS -> pic_width_in_luma_samples << " ";
  out << "pic_height_in_luma_samples: " << pSPS -> pic_height_in_luma_samples << " ";
  out << "conformance_window_flag: " << (int) pSPS -> conformance_window_flag << " ";
  if(pSPS -> conformance_window_flag)
  {
    out << "conformance_window { ";
    out << "conf_win_left_offset: " << pSPS -> conf_win_left_offset << " ";
    out << "conf_win_right_offset: " << pSPS -> conf_win_right_offset << " ";
    out << "conf_win_top_offset: " << pSPS -> conf_win_top_offset << " ";
    out << "conf_win_bottom_offset: " << pSPS -> conf_win_bottom_offset << " ";
    out << "} ";
  }

  out << "bit_depth_luma_minus8: " << pSPS -> bit_depth_luma_minus8 << " ";
  out << "bit_depth_chroma_minus8: " << pSPS -> bit_depth_chroma_minus8 << " ";
  out << "log2_max_pic_order_cnt_lsb_minus4: " << pSPS -> log2_max_pic_order_cnt_lsb_minus4 << " ";
  out << "sps_sub_layer_ordering_info_present_flag: " << (int) pSPS -> sps_sub_layer_ordering_info_present_flag << " ";

  out << "sps_sub_layer_ordering_info { ";
  for(std::size_t i = (pSPS -> sps_sub_layer_ordering_info_present_flag ? 0 : pSPS -> sps_max_sub_layers_minus1); i <= pSPS -> sps_max_sub_layers_minus1; i++ )
  {
    out << "sps_max_dec_pic_buffering_minus1[" << i << "]: " << pSPS -> sps_max_dec_pic_buffering_minus1[i] << " ";
    out << "sps_max_num_reorder_pics[" << i << "]: " << pSPS -> sps_max_num_reorder_pics[i] << " ";
    out << "sps_max_latency_increase_plus1[" << i << "]: " << pSPS -> sps_max_latency_increase_plus1[i] << " ";
  }
  out << "} ";


  out << "log2_min_luma_coding_block_size_minus3: " << pSPS -> log2_min_luma_coding_block_size_minus3 << " ";
  out << "log2_diff_max_min_luma_coding_block_size: " << pSPS -> log2_diff_max_min_luma_coding_block_size << " ";
  out << "log2_min_transform_block_size_minus2: " << pSPS -> log2_min_transform_block_size_minus2 << " ";
  out << "log2_diff_max_min_transform_block_size: " << pSPS -> log2_diff_max_min_transform_block_size << " ";
  out << "max_transform_hierarchy_depth_inter: " << pSPS -> max_transform_hierarchy_depth_inter << " ";
  out << "max_transform_hierarchy_depth_intra: " << pSPS -> max_transform_hierarchy_depth_intra << " ";
  out << "scaling_list_enabled_flag: " << (int) pSPS -> scaling_list_enabled_flag << " ";

  if(pSPS -> scaling_list_enabled_flag)
  {
    out << "scaling_list_enabled { ";
    out << "sps_scaling_list_data_present_flag: " << (int) pSPS -> sps_scaling_list_data_present_flag << " ";
    //scaling_list_data( )
    out << "} ";
  }
  out << "amp_enabled_flag: " << (int) pSPS -> amp_enabled_flag << " ";
  out << "sample_adaptive_offset_enabled_flag: " << (int) pSPS -> sample_adaptive_offset_enabled_flag << " ";
  out << "pcm_enabled_flag: " << (int) pSPS -> pcm_enabled_flag << " ";

  if(pSPS -> pcm_enabled_flag)
  {
    out << "pcm_enabled { ";
    out << "pcm_sample_bit_depth_luma_minus1: " << pSPS -> pcm_sample_bit_depth_luma_minus1 << " ";
    out << "pcm_sample_bit_depth_chroma_minus1: " << pSPS -> pcm_sample_bit_depth_chroma_minus1 << " ";
    out << "log2_min_pcm_luma_coding_block_size_minus3: " << pSPS -> log2_min_pcm_luma_coding_block_size_minus3 << " ";
    out << "log2_diff_max_min_pcm_luma_coding_block_size: " << pSPS -> log2_diff_max_min_pcm_luma_coding_block_size << " ";
    out << "pcm_loop_filter_disabled_flag: " << pSPS -> pcm_loop_filter_disabled_flag << " ";
    out << "} ";
  }

  out << "num_short_term_ref_pic_sets: " << pSPS -> num_short_term_ref_pic_sets << " ";

  out << "short_term_ref_pic_sets { ";
  for(std::size_t i=0; i<pSPS -> num_short_term_ref_pic_sets; i++)
  {
    out << "short_term_ref_pic_set[" << i << "] (" << i << "): ";
    writeShortTermRefPicSet(i, pSPS -> num_short_term_ref_pic_sets, pSPS -> short_term_ref_pic_set, out);
    out << " ";
  }
  out << "} ";

  out << "long_term_ref_pics_present_flag: " << (int) pSPS -> long_term_ref_pics_present_flag << " ";
  if(pSPS -> long_term_ref_pics_present_flag)
  {
    out << "long_term_ref_pics_present { ";
    out << "num_long_term_ref_pics_sps: " << (int) pSPS -> num_long_term_ref_pics_sps << " ";

    for(std::size_t i=0; i<pSPS -> num_long_term_ref_pics_sps; i++)
    {
      out << "lt_ref_pic_poc_lsb_sps[" << i << "]: " << (int) pSPS -> lt_ref_pic_poc_lsb_sps[i] << " ";
      out << "used_by_curr_pic_lt_sps_flag[" << i << "]: " << (int) pSPS -> used_by_curr_pic_lt_sps_flag[i] << " ";
    }
    out << "} ";
  }

  out << "sps_temporal_mvp_enabled_flag: " << (int) pSPS -> sps_temporal_mvp_enabled_flag << " ";
  out << "strong_intra_smoothing_enabled_flag: " << (int) pSPS -> strong_intra_smoothing_enabled_flag << " ";
  out << "vui_parameters_present_flag: " << (int) pSPS -> vui_parameters_present_flag << " ";

  if(pSPS -> vui_parameters_present_flag)
  {
    out << "vui_parameters_present { ";
    out << "vui_parameters(" << (int) pSPS -> sps_max_sub_layers_minus1 << "):";
    writeVuiParameters(pSPS -> vui_parameters, pSPS ->  sps_max_sub_layers_minus1, out);
    out << "} ";
  }

  out << "sps_extension_flag: " << (int) pSPS -> sps_extension_flag << " ";
}


void HEVCInfoAltWriter::writePPS(std::shared_ptr<HEVC::PPS> pPPS, std::ostream &out)
{
  out << "pps_pic_parameter_set_id: " << (int) pPPS -> pps_pic_parameter_set_id << " ";
  out << "pps_seq_parameter_set_id: " << (int) pPPS -> pps_seq_parameter_set_id << " ";
  out << "dependent_slice_segments_enabled_flag: " << (int) pPPS -> dependent_slice_segments_enabled_flag << " ";
  out << "output_flag_present_flag: " << (int) pPPS -> output_flag_present_flag << " ";
  out << "num_extra_slice_header_bits: " << (int) pPPS -> num_extra_slice_header_bits << " ";
  out << "sign_data_hiding_flag: " << (int) pPPS -> sign_data_hiding_flag << " ";
  out << "cabac_init_present_flag: " << (int) pPPS -> cabac_init_present_flag << " ";
  out << "num_ref_idx_l0_default_active_minus1: " << pPPS -> num_ref_idx_l0_default_active_minus1 << " ";
  out << "num_ref_idx_l1_default_active_minus1: " << pPPS -> num_ref_idx_l1_default_active_minus1 << " ";
  out << "init_qp_minus26: " << pPPS -> init_qp_minus26 << " ";
  out << "constrained_intra_pred_flag: " << (int) pPPS -> constrained_intra_pred_flag << " ";
  out << "transform_skip_enabled_flag: " << (int) pPPS -> transform_skip_enabled_flag << " ";
  out << "cu_qp_delta_enabled_flag: " << (int) pPPS -> cu_qp_delta_enabled_flag << " ";
  if(pPPS -> cu_qp_delta_enabled_flag)
  {
    out << "cu_qp_delta { ";
    out << "diff_cu_qp_delta_depth: " << pPPS -> diff_cu_qp_delta_depth << " ";
    out << "} ";
  }

  out << "pps_cb_qp_offset: " << pPPS -> pps_cb_qp_offset << " ";
  out << "pps_cr_qp_offset: " << pPPS -> pps_cr_qp_offset << " ";
  out << "pps_slice_chroma_qp_offsets_present_flag: " << (int) pPPS -> pps_slice_chroma_qp_offsets_present_flag << " ";
  out << "weighted_pred_flag: " << (int) pPPS -> weighted_pred_flag << " ";
  out << "weighted_bipred_flag: " << (int) pPPS -> weighted_bipred_flag << " ";
  out << "transquant_bypass_enabled_flag: " << (int) pPPS -> transquant_bypass_enabled_flag << " ";
  out << "tiles_enabled_flag: " << (int) pPPS -> tiles_enabled_flag << " ";
  out << "entropy_coding_sync_enabled_flag: " << (int) pPPS -> entropy_coding_sync_enabled_flag << " ";

  if(pPPS -> tiles_enabled_flag)
  {
    out << "tiles_enabled { ";

    out << "num_tile_columns_minus1: " << (int) pPPS -> num_tile_columns_minus1 << " ";
    out << "num_tile_rows_minus1: " << (int) pPPS -> num_tile_rows_minus1 << " ";
    out << "uniform_spacing_flag: " << (int) pPPS -> uniform_spacing_flag << " ";

    if(!pPPS -> uniform_spacing_flag)
    {
      out << "not_uniform_spacing { ";

      out << "column_width_minus1 = { ";
      for(std::size_t i=0; i<pPPS -> num_tile_columns_minus1 - 1; i++)
        out << (int)  pPPS -> column_width_minus1[i] << ", ";
      out << (int) pPPS -> column_width_minus1[pPPS -> num_tile_columns_minus1 - 1] << " }";

      out << "num_tile_rows_minus1 = { ";
      for(std::size_t i=0; i<pPPS -> num_tile_rows_minus1 - 1; i++)
        out << (int)  pPPS -> row_height_minus1[i] << ", ";
      out << (int) pPPS -> row_height_minus1[pPPS -> num_tile_rows_minus1 - 1] << " }";
      out << "} ";
    }

    out << "uniform_spacing_flag: " << (int) pPPS -> uniform_spacing_flag;
    out << "} ";
  }

  out << "pps_loop_filter_across_slices_enabled_flag: " << (int) pPPS -> pps_loop_filter_across_slices_enabled_flag << " ";
  out << "deblocking_filter_control_present_flag: " << (int) pPPS -> deblocking_filter_control_present_flag << " ";

  if(pPPS -> deblocking_filter_control_present_flag)
  {
    out << "deblocking_filter_control { ";
    out << "deblocking_filter_override_enabled_flag: " << (int) pPPS -> deblocking_filter_override_enabled_flag << " ";
    out << "pps_deblocking_filter_disabled_flag: " << (int) pPPS -> pps_deblocking_filter_disabled_flag << " ";

    if(!pPPS -> pps_deblocking_filter_disabled_flag)
    {
      out << "not_pps_deblocking_filter_disabled { ";
      out << "pps_beta_offset_div2: " << (int) pPPS -> pps_beta_offset_div2 << " ";
      out << "pps_tc_offset_div2: " << (int) pPPS -> pps_tc_offset_div2 << " ";
      out << "} ";
    }
    out << "} ";
  }
}



void HEVCInfoAltWriter::writeSlice(std::shared_ptr<HEVC::Slice> pSlice, std::ostream &out)
{
  std::shared_ptr<PPS> pPPS = m_ppsMap[pSlice -> slice_pic_parameter_set_id];
  if(!pPPS)
    return;
  int32_t spsId = pPPS -> pps_seq_parameter_set_id;


  out << "first_slice_segment_in_pic_flag: " << (int) pSlice -> first_slice_segment_in_pic_flag << " ";
  if(pSlice -> m_nalUnitType >= HEVC::NAL_BLA_W_LP && pSlice -> m_nalUnitType <= HEVC::NAL_IRAP_VCL23)
  {
    out << "no_output_of_prior_pics_flag: " << (int) pSlice -> no_output_of_prior_pics_flag << " ";
  }

  out << "slice_pic_parameter_set_id: " << pSlice -> slice_pic_parameter_set_id << " ";


  if(!pSlice -> first_slice_segment_in_pic_flag)
  {
    out << "not_first_slice_segment_in_pic { ";

    if(pPPS -> dependent_slice_segments_enabled_flag)
    {
      out << "dependent_slice_segments_enabled { ";
      out << "dependent_slice_segment_flag: " << (int)pSlice -> dependent_slice_segment_flag << " ";
      out << "} ";
    }

    out << "slice_segment_address: " << pSlice -> slice_segment_address << " ";
    out << "} ";
  }


  if(!pSlice -> dependent_slice_segment_flag)
  {
    out << "not_dependent_slice_segment_flag { ";
    long num_extra_slice_header_bits = pPPS -> num_extra_slice_header_bits;
    out << "slice_reserved_undetermined_flag { ";

    if(num_extra_slice_header_bits > 0)
    {
      out << "";
      for(long i=0; i<num_extra_slice_header_bits - 1; i++)
        out << pSlice -> slice_reserved_undetermined_flag[i] << ", ";
      out << pSlice -> slice_reserved_undetermined_flag[num_extra_slice_header_bits - 1];
    }
    out << "} ";

    out << "slice_type: " << pSlice -> slice_type << " ";

    if(pPPS -> output_flag_present_flag)
    {
      out << "output_flag_present { ";
      out << "pic_output_flag: " << (int) pSlice -> pic_output_flag << " ";
      out << "} ";
    }

    if(m_spsMap[spsId] -> separate_colour_plane_flag)
    {
      out << "separate_colour_plane { ";
      out << "colour_plane_id: " << (int) pSlice -> colour_plane_id << " ";
      out << "} ";
    }

    bool IdrPicFlag = pSlice -> m_nalUnitType == NAL_IDR_W_RADL || pSlice -> m_nalUnitType == NAL_IDR_N_LP;
    if(!IdrPicFlag)
    {
      out << "slice_pic_order_cnt_lsb: " << pSlice -> slice_pic_order_cnt_lsb << " ";
      out << "short_term_ref_pic_set_sps_flag: " << (int) pSlice -> short_term_ref_pic_set_sps_flag << " ";

      if(!pSlice -> short_term_ref_pic_set_sps_flag)
      {
        out << "not_short_term_ref_pic_set_sps { ";
        //short_term_ref_pic_set
        out << "} ";
      }
      else if(pSlice -> short_term_ref_pic_set_sps_flag > 1)
      {
        std::size_t numBits = HEVC::log2(m_spsMap[spsId] -> num_short_term_ref_pic_sets);
        if(numBits > 0)
          out << "short_term_ref_pic_set_idx: " << (int) pSlice -> short_term_ref_pic_set_idx << " ";
      }
    }
    out << "} ";
  }
}

void HEVCInfoAltWriter::writeAUD(std::shared_ptr<HEVC::AUD> pAUD, std::ostream &out)
{
  out << "pic_type: " << (int) pAUD -> pic_type << " ";
}

void HEVCInfoAltWriter::writeSEI(std::shared_ptr<HEVC::SEI> pSEI, std::ostream &out)
{
  out << "sei_rbsp { ";
  std::size_t payloadType = 0;
  std::size_t payloadSize = 0;
  for(std::size_t i=0; i< pSEI -> sei_message.size(); i++)
  {
    out << "sei_message { ";
    if(pSEI -> sei_message[i].num_payload_type_ff_bytes)
    {
      for(std::size_t i=0; i<pSEI -> sei_message[i].num_payload_type_ff_bytes; i++)
      {
        payloadType += 255;
      }
    }
    payloadType += pSEI -> sei_message[i].last_payload_type_byte;
    out << "payload_type: " << payloadType << " ";

    if(pSEI -> sei_message[i].num_payload_size_ff_bytes)
    {
      for(std::size_t j=0; j<pSEI -> sei_message[i].num_payload_size_ff_bytes; j++)
      {
        payloadSize += 255;
      }
    }

    payloadSize += pSEI -> sei_message[i].last_payload_size_byte;
    out << "payload_size: " << payloadSize << " ";
    //out << "sei_payload { ";
    //out << "} ";
    out << "} ";
  }

  out << " }";
}


void HEVCInfoAltWriter::writeProfileTierLevel(const ProfileTierLevel &ptl, std::ostream &out)
{
  out << "general_profile_space: " << (int) ptl.general_profile_space << " ";
  out << "general_tier_flag: " << (int) ptl.general_tier_flag << " ";
  out << "general_profile_idc: " << (int) ptl.general_profile_idc << " ";

  out << "general_profile_compatibility_flag[i] = { ";
  for(std::size_t i=0; i<31; i++)
    out << (int) ptl.general_profile_compatibility_flag[i] << ", ";
  out << (int) ptl.general_profile_compatibility_flag[31] << " } ";;

  out << "general_progressive_source_flag: " << (int) ptl.general_progressive_source_flag << " ";
  out << "general_interlaced_source_flag: " << (int) ptl.general_interlaced_source_flag << " ";
  out << "general_non_packed_constraint_flag: " << (int) ptl.general_non_packed_constraint_flag << " ";
  out << "general_frame_only_constraint_flag: " << (int) ptl.general_frame_only_constraint_flag << " ";
  out << "general_level_idc: " << (int) ptl.general_level_idc << " ";

  std::size_t maxNumSubLayersMinus1 = ptl.sub_layer_profile_present_flag.size();

  if(maxNumSubLayersMinus1 != 0)
  {
    out << "sub_layer_profile_present_flag { ";
    for(std::size_t i=0; i<maxNumSubLayersMinus1 - 1; i++)
      out << (int) ptl.sub_layer_profile_present_flag[i] << ", ";
    out << (int) ptl.sub_layer_profile_present_flag[maxNumSubLayersMinus1 - 1];
    out << "} ";

    out << "sub_layer_level_present_flag { ";
    for(std::size_t i=0; i<maxNumSubLayersMinus1 - 1; i++)
      out << (int) ptl.sub_layer_level_present_flag[i] << ", ";
    out << (int) ptl.sub_layer_level_present_flag[maxNumSubLayersMinus1 - 1];
    out << "} ";
  }

  for(std::size_t i=0; i<maxNumSubLayersMinus1; i++)
  {
    if(ptl.sub_layer_profile_present_flag[i])
    {
      out << "sub_layer_profile_present_flag { ";
      out << "sub_layer_profile_space[" << i << "]: " << (int) ptl.sub_layer_profile_space[i];
      out << "sub_layer_tier_flag[" << i << "]: " << (int) ptl.sub_layer_tier_flag[i];
      out << "sub_layer_profile_idc[" << i << "]: " << (int) ptl.sub_layer_profile_idc[i];
      out << "sub_layer_profile_compatibility_flag[" << i << "] = {";
      for(std::size_t j=0; j<31; j++)
        out << (int) ptl.sub_layer_profile_compatibility_flag[i][j] << ", ";
      out << (int) ptl.sub_layer_profile_compatibility_flag[i][31] << " }";

      out << "sub_layer_progressive_source_flag[" << i << "]: " << (int) ptl.sub_layer_progressive_source_flag[i];
      out << "sub_layer_interlaced_source_flag[" << i << "]: " << (int) ptl.sub_layer_interlaced_source_flag[i];
      out << "sub_layer_non_packed_constraint_flag[" << i << "]: " << (int) ptl.sub_layer_non_packed_constraint_flag[i];
      out << "sub_layer_frame_only_constraint_flag[" << i << "]: " << (int) ptl.sub_layer_frame_only_constraint_flag[i];
      out << "} ";
    }

    if(ptl.sub_layer_level_present_flag[i])
    {
      out << "if( sub_layer_level_present_flag[ i ] )";
      out << "sub_layer_level_idc[" << i << "]: " << (int) ptl.sub_layer_level_idc[i];
    }
  }
}


void HEVCInfoAltWriter::writeHrdParameters(const HEVC::HrdParameters &hrd, uint8_t commonInfPresentFlag, std::ostream &out)
{
  if(commonInfPresentFlag)
  {
    out << "if( commonInfPresentFlag )";
    out << "nal_hrd_parameters_present_flag: " << (int) hrd.nal_hrd_parameters_present_flag;
    out << "vcl_hrd_parameters_present_flag: " << (int) hrd.vcl_hrd_parameters_present_flag;

    if(hrd.nal_hrd_parameters_present_flag || hrd.vcl_hrd_parameters_present_flag)
    {
      out << "";
      out << "if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag) ";

      out << "sub_pic_hrd_params_present_flag: " << (int) hrd.sub_pic_hrd_params_present_flag;
      if(hrd.sub_pic_hrd_params_present_flag)
      {
        out << "if( sub_pic_hrd_params_present_flag )";
        out << "tick_divisor_minus2: " << (int) hrd.tick_divisor_minus2;
        out << "du_cpb_removal_delay_increment_length_minus1: " << (int) hrd.du_cpb_removal_delay_increment_length_minus1;
        out << "sub_pic_cpb_params_in_pic_timing_sei_flag: " << (int) hrd.sub_pic_cpb_params_in_pic_timing_sei_flag;
        out << "dpb_output_delay_du_length_minus1: " << (int) hrd.dpb_output_delay_du_length_minus1;
      }

      out << "bit_rate_scale: " << (int) hrd.bit_rate_scale;
      out << "cpb_size_scale: " << (int) hrd.cpb_size_scale;

      if(hrd.sub_pic_hrd_params_present_flag)
      {
        out << "if (sub_pic_hrd_params_present_flag )";
        out << "cpb_size_du_scale: " << (int) hrd.cpb_size_du_scale;
      }

      out << "initial_cpb_removal_delay_length_minus1: " << (int) hrd.initial_cpb_removal_delay_length_minus1;
      out << "au_cpb_removal_delay_length_minus1: " << (int) hrd.au_cpb_removal_delay_length_minus1;
      out << "dpb_output_delay_length_minus1: " << (int) hrd.dpb_output_delay_length_minus1;
    }
  }

  out << "for( i = 0; i <= maxNumSubLayersMinus1; i++ )";
  for(std::size_t i = 0; i < hrd.fixed_pic_rate_general_flag.size(); i++ )
  {
    out << "fixed_pic_rate_general_flag[" << i << "]: " << (int) hrd.fixed_pic_rate_general_flag[i];
    if(!hrd.fixed_pic_rate_general_flag[i])
    {
      out <<  "if( !fixed_pic_rate_general_flag[ i ] )";
      out << "fixed_pic_rate_within_cvs_flag[" << i << "]: " << (int) hrd.fixed_pic_rate_within_cvs_flag[i];
    }

    if(hrd.fixed_pic_rate_within_cvs_flag[i])
    {
      out << "if( fixed_pic_rate_within_cvs_flag[ i ] )";
      out << "elemental_duration_in_tc_minus1[" << i << "]: " << (int) hrd.elemental_duration_in_tc_minus1[i];
    }
    else
    {
      out <<  "if( !fixed_pic_rate_within_cvs_flag[ i ] )";
      out << "low_delay_hrd_flag[" << i << "]: " << (int) hrd.low_delay_hrd_flag[i];
    }

    if(!hrd.low_delay_hrd_flag[i])
    {
      out <<  "if( !low_delay_hrd_flag[ i ] )";
      out << "cpb_cnt_minus1[" << i << "]: " << (int) hrd.cpb_cnt_minus1[i];
    }

    if(hrd.nal_hrd_parameters_present_flag)
    {
      out <<  "if( nal_hrd_parameters_present_flag )";
      out << "sub_layer_hrd_parameters(" << i << ")";
      writeSubLayerHrdParameters(hrd.nal_sub_layer_hrd_parameters[i], i,out);
    }
    if(hrd.vcl_hrd_parameters_present_flag)
    {
      out <<  "if( vcl_hrd_parameters_present_flag )";
      out << "sub_layer_hrd_parameters(" << i << ")";
      writeSubLayerHrdParameters(hrd.vcl_sub_layer_hrd_parameters[i], i, out);

    }
  }
}


void HEVCInfoAltWriter::writeSubLayerHrdParameters(const HEVC::SubLayerHrdParameters &slhrd, uint8_t sub_pic_hrd_params_present_flag, std::ostream &out)
{

  out << "for( i = 0; i <= CpbCnt; i++ )";

  for(std::size_t i=0; i<slhrd.bit_rate_value_minus1.size(); i++)
  {
    out << "bit_rate_value_minus1[" << i << "]: " << (int) slhrd.bit_rate_value_minus1[i];
    out << "cpb_size_value_minus1[" << i << "]: " << (int) slhrd.cpb_size_value_minus1[i];

    if(sub_pic_hrd_params_present_flag)
    {
      out << "if( sub_pic_hrd_params_present_flag )";
      out << "cpb_size_du_value_minus1[" << i << "]: " << (int) slhrd.cpb_size_du_value_minus1[i];
      out << "bit_rate_du_value_minus1[" << i << "]: " << (int) slhrd.bit_rate_du_value_minus1[i];
    }

    out << "cbr_flag[" << i << "]: " << (int) slhrd.cbr_flag[i];
  }

}


void HEVCInfoAltWriter::writeShortTermRefPicSet(std::size_t stRpsIdx, std::size_t num_short_term_ref_pic_sets, const std::vector<ShortTermRefPicSet> &refPicSets, std::ostream &out)
{
  ShortTermRefPicSet rpset = refPicSets[stRpsIdx];

  if(stRpsIdx)
  {
    out << "if( stRpsIdx != 0 )";
    out << "inter_ref_pic_set_prediction_flag: " << (int) refPicSets[stRpsIdx].inter_ref_pic_set_prediction_flag;
  }

  if(rpset.inter_ref_pic_set_prediction_flag)
  {
    out << "if( inter_ref_pic_set_prediction_flag )";

    if(stRpsIdx == num_short_term_ref_pic_sets)
    {
      out << "if( stRpsIdx == num_short_term_ref_pic_sets )";
      out << "delta_idx_minus1: " << (int) refPicSets[stRpsIdx].delta_idx_minus1;
    }

    out << "delta_rps_sign: " << (int) refPicSets[stRpsIdx].delta_rps_sign;
    out << "abs_delta_rps_minus1: " << (int) refPicSets[stRpsIdx].abs_delta_rps_minus1;

    std::size_t RefRpsIdx = stRpsIdx - (rpset.delta_idx_minus1+1);
    std::size_t NumDeltaPocs = refPicSets[RefRpsIdx].num_negative_pics + refPicSets[RefRpsIdx].num_negative_pics;

    out << "for( j = 0; j <= NumDeltaPocs[ RefRpsIdx ]; j++ )";
    for(std::size_t i=0; i<=NumDeltaPocs; i++ )
    {
      out << "used_by_curr_pic_flag [" << i << "]: " << (int) refPicSets[stRpsIdx].used_by_curr_pic_flag[i];
      if(!rpset.used_by_curr_pic_flag[i])
      {
        out << "if( !used_by_curr_pic_flag[j] )";
        out << "use_delta_flag [" << i << "]: " << (int) refPicSets[stRpsIdx].use_delta_flag[i];
      }
    }
  }
  else
  {
    out << "if( !inter_ref_pic_set_prediction_flag )";

    out << "num_negative_pics: " << (int) refPicSets[stRpsIdx].num_negative_pics;
    out << "num_positive_pics: " << (int) refPicSets[stRpsIdx].num_positive_pics;

    out << "for( i = 0; i < num_negative_pics; i++ )";
    for(std::size_t i=0; i<rpset.num_negative_pics; i++)
    {
      out << "delta_poc_s0_minus1[" << i <<  "]: " << (int) refPicSets[stRpsIdx].delta_poc_s0_minus1[i];
      out << "used_by_curr_pic_s0_flag[" << i <<  "]: " << (int) refPicSets[stRpsIdx].used_by_curr_pic_s0_flag[i];
    }

    out << "for( i = 0; i < num_positive_pics; i++ )";
    for(std::size_t i=0; i<rpset.num_positive_pics; i++)
    {
      out << "delta_poc_s1_minus1[" << i <<  "]: " << (int) refPicSets[stRpsIdx].delta_poc_s1_minus1[i];
      out << "used_by_curr_pic_s1_flag[" << i <<  "]: " << (int) refPicSets[stRpsIdx].used_by_curr_pic_s1_flag[i];
    }

  }
}


void HEVCInfoAltWriter::writeVuiParameters(const HEVC::VuiParameters &vui, std::size_t sps_max_sub_layers_minus1, std::ostream &out)
{
  out << "aspect_ratio_info_present_flag: " << (int) vui.aspect_ratio_info_present_flag << " ";

  if(vui.aspect_ratio_info_present_flag)
  {
    out << "aspect_ratio_info { ";
    out << "aspect_ratio_idc: " << (int) vui.aspect_ratio_idc << " ";

    if(vui.aspect_ratio_idc == 255) //EXTENDED_SAR
    {
      //out <<  "if( aspect_ratio_idc = = EXTENDED_SAR )";
      out << "sar_width: " << (int) vui.sar_width << " ";
      out << "sar_height: " << (int) vui.sar_height << " ";
    }
    out << "} ";
  }

  out << "overscan_info_present_flag: " << (int) vui.overscan_info_present_flag << " ";
  if(vui.overscan_info_present_flag)
  {
    out << "overscan_info { ";
    out << "overscan_appropriate_flag: " << (int) vui.overscan_appropriate_flag << " ";
    out << "} ";
  }

  out << "video_signal_type_present_flag: " << (int) vui.video_signal_type_present_flag << " ";

  if(vui.video_signal_type_present_flag)
  {
    out << "video_signal_type { ";
    out << "video_format: " << (int) vui.video_format << " ";
    out << "video_full_range_flag: " << (int) vui.video_full_range_flag << " ";
    out << "colour_description_present_flag: " << (int) vui.colour_description_present_flag << " ";
    out << "} ";

    if(vui.colour_description_present_flag)
    {
      out << "colour_description { ";
      out << "colour_primaries: " << (int) vui.colour_primaries << " ";
      out << "transfer_characteristics: " << (int) vui.transfer_characteristics << " ";
      out << "matrix_coeffs: " << (int) vui.matrix_coeffs << " ";
      out << "} ";
    }

  }

  out << "chroma_loc_info_present_flag: " << (int) vui.chroma_loc_info_present_flag << " ";

  if(vui.chroma_loc_info_present_flag)
  {
    out << "chroma_loc_info { ";
    out << "chroma_sample_loc_type_top_field: " << (int) vui.chroma_sample_loc_type_top_field << " ";
    out << "chroma_sample_loc_type_bottom_field: " << (int) vui.chroma_sample_loc_type_bottom_field << " ";
    out << "} ";
  }

  out << "neutral_chroma_indication_flag: " << (int) vui.neutral_chroma_indication_flag << " ";
  out << "field_seq_flag: " << (int) vui.field_seq_flag << " ";
  out << "frame_field_info_present_flag: " << (int) vui.frame_field_info_present_flag << " ";
  out << "default_display_window_flag: " << (int) vui.default_display_window_flag << " ";

  if(vui.default_display_window_flag)
  {
    out << "default_display_window { ";
    out << "def_disp_win_left_offset: " << (int) vui.def_disp_win_left_offset << " ";
    out << "def_disp_win_right_offset: " << (int) vui.def_disp_win_right_offset << " ";
    out << "def_disp_win_top_offset: " << (int) vui.def_disp_win_top_offset << " ";
    out << "def_disp_win_bottom_offset: " << (int) vui.def_disp_win_bottom_offset << " ";
    out << "} ";
  }

  out << "vui_timing_info_present_flag: " << (int) vui.vui_timing_info_present_flag << " ";

  if(vui.vui_timing_info_present_flag)
  {
    out << "vui_timing_info { ";
    out << "vui_num_units_in_tick: " << (int) vui.vui_num_units_in_tick << " ";
    out << "vui_time_scale: " << (int) vui.vui_time_scale << " ";
    out << "vui_poc_proportional_to_timing_flag: " << (int) vui.vui_poc_proportional_to_timing_flag << " ";

    if(vui.vui_poc_proportional_to_timing_flag)
    {
      out << "vui_poc_proportional_to_timing { ";
      out << "vui_num_ticks_poc_diff_one_minus1: " << (int) vui.vui_num_ticks_poc_diff_one_minus1 << " ";
      out << "} ";
    }

    out << "vui_hrd_parameters_present_flag: " << (int) vui.vui_hrd_parameters_present_flag << " ";

    if(vui.vui_hrd_parameters_present_flag)
    {
      out <<  "vui_hrd_parameters { ";
      out << "hrd_parameters(" << 1 << ", " << sps_max_sub_layers_minus1 << "): ";
      writeHrdParameters(vui.hrd_parameters, 1, out);
      out << "} ";
    }
    out << "} ";
  }

  out << "bitstream_restriction_flag: " << (int) vui.bitstream_restriction_flag << " ";

  if(vui.bitstream_restriction_flag)
  {
    out << "if( bitstream_restriction_flag )";
    out << "tiles_fixed_structure_flag: " << (int) vui.tiles_fixed_structure_flag << " ";
    out << "motion_vectors_over_pic_boundaries_flag: " << (int) vui.motion_vectors_over_pic_boundaries_flag << " ";
    out << "restricted_ref_pic_lists_flag: " << (int) vui.restricted_ref_pic_lists_flag << " ";
    out << "min_spatial_segmentation_idc: " << (int) vui.min_spatial_segmentation_idc << " ";
    out << "max_bytes_per_pic_denom: " << (int) vui.max_bytes_per_pic_denom << " ";
    out << "max_bits_per_min_cu_denom: " << (int) vui.max_bits_per_min_cu_denom << " ";
    out << "log2_max_mv_length_horizontal: " << (int) vui.log2_max_mv_length_horizontal << " ";
    out << "log2_max_mv_length_vertical: " << (int) vui.log2_max_mv_length_vertical << " ";
  }
}
