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
    out << std::hex << "0x" << m_nalus[i].m_info.m_position << std::dec << ": ";
    out << ConvToString::NALUnitType(m_nalus[i].m_pNALUnit -> m_nalUnitType) << std::endl;

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
    };
    out << std::endl;
  }
}


void HEVCInfoWriter::writeVPS(std::shared_ptr<HEVC::VPS> pVPS, std::ostream &out)
{
  out << "\t" << "vps_video_parameter_set_id = " << (int) pVPS -> vps_video_parameter_set_id << std::endl;
  out << "\t" << "vps_max_layers_minus1 = " << (int) pVPS -> vps_max_layers_minus1 << std::endl;
  out << "\t" << "vps_max_sub_layers_minus1 = " << (int) pVPS -> vps_max_sub_layers_minus1 << std::endl;
  out << "\t" << "vps_temporal_id_nesting_flag = " << (int) pVPS -> vps_temporal_id_nesting_flag << std::endl;
}


void HEVCInfoWriter::writeSPS(std::shared_ptr<HEVC::SPS> pSPS, std::ostream &out)
{
  out << "\t" << "sps_video_parameter_set_id = " << (int) pSPS -> sps_video_parameter_set_id << std::endl;
  out << "\t" << "sps_max_sub_layers_minus1 = " << (int) pSPS -> sps_max_sub_layers_minus1 << std::endl;
  out << "\t" << "sps_temporal_id_nesting_flag = " << (int) pSPS -> sps_temporal_id_nesting_flag << std::endl;


  out << "\t" << "profile_tier_level:";
  out << "\t\t" << "general_profile_space = " << (int) pSPS -> profile_tier_level.general_profile_space << std::endl;
  out << "\t\t" << "general_tier_flag = " << (int) pSPS -> profile_tier_level.general_tier_flag << std::endl;
  out << "\t\t" << "general_profile_idc = " << (int) pSPS -> profile_tier_level.general_profile_idc << std::endl;

  out << "\t\t" << "general_profile_compatibility_flag[i] = { ";
  for(std::size_t i=0; i<31; i++)
    out << (int) pSPS -> profile_tier_level.general_profile_compatibility_flag[i] << ", ";
  out << (int) pSPS -> profile_tier_level.general_profile_compatibility_flag[31] << " }" << std::endl;;

  out << "\t\t" << "general_progressive_source_flag = " << (int) pSPS -> profile_tier_level.general_progressive_source_flag << std::endl;
  out << "\t\t" << "general_interlaced_source_flag = " << (int) pSPS -> profile_tier_level.general_interlaced_source_flag << std::endl;
  out << "\t\t" << "general_non_packed_constraint_flag = " << (int) pSPS -> profile_tier_level.general_non_packed_constraint_flag << std::endl;
  out << "\t\t" << "general_frame_only_constraint_flag = " << (int) pSPS -> profile_tier_level.general_frame_only_constraint_flag << std::endl;
  out << "\t\t" << "general_level_idc = " << (int) pSPS -> profile_tier_level.general_level_idc << std::endl;

  if(pSPS -> sps_max_sub_layers_minus1 == 0)
  {
    out << "\t\t" << "sub_layer_profile_present_flag = {}" << std::endl;
    out << "\t\t" << "sub_layer_level_present_flag = {}" << std::endl;
  }
  else
  {
    out << "\t\t" << "sub_layer_profile_present_flag = { " << std::endl;
    for(std::size_t i=0; i<pSPS -> sps_max_sub_layers_minus1 - 1; i++)
      out << (int) pSPS -> profile_tier_level.sub_layer_profile_present_flag[i] << ", ";
    out << (int) pSPS -> profile_tier_level.sub_layer_profile_present_flag[pSPS -> sps_max_sub_layers_minus1 - 1] << " }" << std::endl;

    out << "\t\t" << "sub_layer_level_present_flag = { " << std::endl;
    for(std::size_t i=0; i<pSPS -> sps_max_sub_layers_minus1 - 1; i++)
      out << (int) pSPS -> profile_tier_level.sub_layer_level_present_flag[i] << ", ";
    out << (int) pSPS -> profile_tier_level.sub_layer_level_present_flag[pSPS -> sps_max_sub_layers_minus1 - 1] << " }" << std::endl;
  }



  out << "\t" << "sps_seq_parameter_set_id = " << pSPS -> sps_seq_parameter_set_id << std::endl;
  out << "\t" << "chroma_format_idc = " << pSPS -> chroma_format_idc << std::endl;
  if(pSPS -> chroma_format_idc == 3)
    out << "\t\t" << "separate_colour_plane_flag = " << (int) pSPS -> separate_colour_plane_flag << std::endl;
  out << "\t" << "pic_width_in_luma_samples = " << pSPS -> pic_width_in_luma_samples << std::endl;
  out << "\t" << "pic_height_in_luma_samples = " << pSPS -> pic_height_in_luma_samples << std::endl;
  out << "\t" << "conformance_window_flag = " << (int) pSPS -> conformance_window_flag << std::endl;
  if(pSPS -> conformance_window_flag)
  {
    out << "\t\t" << "conf_win_left_offset = " << pSPS -> conf_win_left_offset << std::endl;
    out << "\t\t" << "conf_win_right_offset = " << pSPS -> conf_win_right_offset << std::endl;
    out << "\t\t" << "conf_win_top_offset = " << pSPS -> conf_win_top_offset << std::endl;
    out << "\t\t" << "conf_win_bottom_offset = " << pSPS -> conf_win_bottom_offset << std::endl;
  }

  out << "\t" << "bit_depth_luma_minus8 = " << pSPS -> bit_depth_luma_minus8 << std::endl;
  out << "\t" << "bit_depth_chroma_minus8 = " << pSPS -> bit_depth_chroma_minus8 << std::endl;
  out << "\t" << "log2_max_pic_order_cnt_lsb_minus4 = " << pSPS -> log2_max_pic_order_cnt_lsb_minus4 << std::endl;
  out << "\t" << "sps_sub_layer_ordering_info_present_flag = " << (int) pSPS -> sps_sub_layer_ordering_info_present_flag << std::endl;
  for(std::size_t i = (pSPS -> sps_sub_layer_ordering_info_present_flag ? 0 : pSPS -> sps_max_sub_layers_minus1); i <= pSPS -> sps_max_sub_layers_minus1; i++ ) 
  {
    out << "\t\t" << "sps_max_dec_pic_buffering_minus1[" << i << "] = " << pSPS -> sps_max_dec_pic_buffering_minus1[i] << std::endl;
    out << "\t\t" << "sps_max_num_reorder_pics[" << i << "] = " << pSPS -> sps_max_num_reorder_pics[i] << std::endl;
    out << "\t\t" << "sps_max_latency_increase[" << i << "] = " << pSPS -> sps_max_latency_increase[i] << std::endl;
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
    out << "\t\t" << "sps_scaling_list_data_present_flag = " << (int) pSPS -> sps_scaling_list_data_present_flag << std::endl;
    //scaling_list_data( )
  }
  out << "\t" << "amp_enabled_flag = " << (int) pSPS -> amp_enabled_flag << std::endl;
  out << "\t" << "sample_adaptive_offset_enabled_flag = " << (int) pSPS -> sample_adaptive_offset_enabled_flag << std::endl;
  out << "\t" << "pcm_enabled_flag = " << (int) pSPS -> pcm_enabled_flag << std::endl;
  out << "\t" << "num_short_term_ref_pic_sets = " << pSPS -> num_short_term_ref_pic_sets << std::endl;
}


void HEVCInfoWriter::writePPS(std::shared_ptr<HEVC::PPS> pPPS, std::ostream &out)
{
  out << "\t" << "pps_pic_parameter_set_id = " << pPPS -> pps_pic_parameter_set_id << std::endl;
  out << "\t" << "pps_seq_parameter_set_id = " << pPPS -> pps_seq_parameter_set_id << std::endl;
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
    out << "\t\t" << "diff_cu_qp_delta_depth = " << pPPS -> diff_cu_qp_delta_depth << std::endl;

  out << "\t" << "pps_cb_qp_offset = " << pPPS -> pps_cb_qp_offset << std::endl;
  out << "\t" << "pps_cr_qp_offset = " << pPPS -> pps_cr_qp_offset << std::endl;
  out << "\t" << "pps_slice_chroma_qp_offsets_present_flag = " << (int) pPPS -> pps_slice_chroma_qp_offsets_present_flag << std::endl;
  out << "\t" << "weighted_pred_flag = " << (int) pPPS -> weighted_pred_flag << std::endl;
  out << "\t" << "weighted_bipred_flag = " << (int) pPPS -> weighted_bipred_flag << std::endl;
  out << "\t" << "transquant_bypass_enabled_flag = " << (int) pPPS -> transquant_bypass_enabled_flag << std::endl;
  out << "\t" << "tiles_enabled_flag = " << (int) pPPS -> tiles_enabled_flag << std::endl;
  out << "\t" << "entropy_coding_sync_enabled_flag = " << (int) pPPS -> entropy_coding_sync_enabled_flag << std::endl;
}



void HEVCInfoWriter::writeSlice(std::shared_ptr<HEVC::Slice> pSlice, std::ostream &out)
{
  std::shared_ptr<PPS> pPPS = m_ppsMap[pSlice -> slice_pic_parameter_set_id];
  int32_t spsId = pPPS -> pps_seq_parameter_set_id;


  out << "\t" << "first_slice_segment_in_pic_flag = " << (int) pSlice -> first_slice_segment_in_pic_flag << std::endl;
  if(pSlice -> m_nalUnitType >= HEVC::NAL_BLA_W_LP && pSlice -> m_nalUnitType <= HEVC::NAL_IRAP_VCL23)
    out << "\t\t" << "no_output_of_prior_pics_flag = " << (int) pSlice -> no_output_of_prior_pics_flag << std::endl;
  out << "\t" << "slice_pic_parameter_set_id = " << pSlice -> slice_pic_parameter_set_id << std::endl;

  if(!pSlice -> first_slice_segment_in_pic_flag)
  {
    if(pPPS -> dependent_slice_segments_enabled_flag)
    {
      out << "\t\t\t" << "dependent_slice_segment_flag = " << (int)pSlice -> dependent_slice_segment_flag << std::endl;
    }

    out << "\t\t" << "slice_segment_address = " << pSlice -> slice_segment_address << std::endl;
  }


  if(!pSlice -> dependent_slice_segment_flag)
  {
    long num_extra_slice_header_bits = pPPS -> num_extra_slice_header_bits;
    out << "\t" << "slice_reserved_undetermined_flag = { ";

    if(num_extra_slice_header_bits > 0)
    {
      for(long i=0; i<num_extra_slice_header_bits - 1; i++)
        out << pSlice -> slice_reserved_undetermined_flag[i] << ", ";
      out << pSlice -> slice_reserved_undetermined_flag[num_extra_slice_header_bits - 1] << std::endl;
    }
    out << " }" << std::endl;

    out << "\t" << "slice_type = " << pSlice -> slice_type << std::endl;

    if(pPPS -> output_flag_present_flag)
      out << "\t\t" << "pic_output_flag = " << (int) pSlice -> pic_output_flag << std::endl;

    if(m_spsMap[spsId] -> separate_colour_plane_flag)
      out << "\t\t" << "colour_plane_id = " << (int) pSlice -> colour_plane_id << std::endl;
    
    bool IdrPicFlag = pSlice -> m_nalUnitType == NAL_IDR_W_RADL || pSlice -> m_nalUnitType == NAL_IDR_N_LP;
    if(!IdrPicFlag)
    {
      out << "\t\t" << "pic_order_cnt_lsb = " << pSlice -> pic_order_cnt_lsb << std::endl;
      out << "\t\t" << "short_term_ref_pic_set_sps_flag = " << (int) pSlice -> short_term_ref_pic_set_sps_flag << std::endl;

      if(!pSlice -> short_term_ref_pic_set_sps_flag)
      {
        //short_term_ref_pic_set
      }
      else if(pSlice -> short_term_ref_pic_set_sps_flag > 1)
      {
        std::size_t numBits = HEVC::log2(m_spsMap[spsId] -> num_short_term_ref_pic_sets);
        if(numBits > 0)
          out << "\t\t\t" << "short_term_ref_pic_set_idx = " << (int) pSlice -> short_term_ref_pic_set_idx << std::endl;
      }
    }
  }
}