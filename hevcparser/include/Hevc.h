#ifndef HEVC_H_
#define HEVC_H_

#include <memory>
#include <vector>
#include <cstdint>
#include <cstddef>

namespace HEVC
{

  enum NALUnitType 
  {
    NAL_TRAIL_N    = 0,
    NAL_TRAIL_R    = 1,
    NAL_TSA_N      = 2,
    NAL_TSA_R      = 3,
    NAL_STSA_N     = 4,
    NAL_STSA_R     = 5,
    NAL_RADL_N     = 6,
    NAL_RADL_R     = 7,
    NAL_RASL_N     = 8,
    NAL_RASL_R     = 9,
    NAL_BLA_W_LP   = 16,
    NAL_BLA_W_RADL = 17,
    NAL_BLA_N_LP   = 18,
    NAL_IDR_W_RADL = 19,
    NAL_IDR_N_LP   = 20,
    NAL_CRA_NUT    = 21,
    NAL_IRAP_VCL23 = 23, 
    NAL_VPS        = 32,
    NAL_SPS        = 33,
    NAL_PPS        = 34,
    NAL_AUD        = 35,
    NAL_EOS_NUT    = 36,
    NAL_EOB_NUT    = 37,
    NAL_FD_NUT     = 38,
    NAL_SEI_PREFIX = 39,
    NAL_SEI_SUFFIX = 40,
  };

  struct ProfileTierLevel
  {
    uint8_t                general_profile_space;
    uint8_t                general_tier_flag;
    uint8_t                general_profile_idc;
    uint8_t                general_profile_compatibility_flag[32];
    uint8_t                general_progressive_source_flag;
    uint8_t                general_interlaced_source_flag;
    uint8_t                general_non_packed_constraint_flag;
    uint8_t                general_frame_only_constraint_flag;
    uint8_t                general_level_idc;
    std::vector<uint8_t>   sub_layer_profile_present_flag;
    std::vector<uint8_t>   sub_layer_level_present_flag;
  };

  
  struct ShortTermRefPicSet
  {
    uint8_t                   inter_ref_pic_set_prediction_flag;
    uint32_t                  delta_idx_minus1;
    uint8_t                   delta_rps_sign;
    uint32_t                  abs_delta_rps_minus1;
    std::vector<uint8_t>      used_by_curr_pic_flag;
    std::vector<uint8_t>      use_delta_flag;
    uint32_t                  num_negative_pics;
    uint32_t                  num_positive_pics;
    std::vector<uint32_t>     delta_poc_s0_minus1;
    std::vector<uint8_t>      used_by_curr_pic_s0_flag;
    std::vector<uint32_t>     delta_poc_s1_minus1;
    std::vector<uint8_t>      used_by_curr_pic_s1_flag;
  };
  
  
  class NALUnit
  {
    public:
      NALUnit(NALUnitType type);
      virtual ~NALUnit();
      virtual NALUnitType getType() const;
      
      std::shared_ptr<NALUnit> copy() const;

      NALUnitType     m_nalUnitType;
  };



  class VPS: public NALUnit
  {
    public:
      VPS(): NALUnit(NAL_VPS) {};
      uint8_t        vps_video_parameter_set_id;
      uint8_t        vps_max_layers_minus1;
      uint8_t        vps_max_sub_layers_minus1;
      uint8_t        vps_temporal_id_nesting_flag;

      bool operator == (const VPS &) const;
  };

  
  class SPS: public NALUnit
  {
    public:
      SPS(): NALUnit(NAL_SPS) {};
      uint8_t                  sps_video_parameter_set_id;
      uint8_t                  sps_max_sub_layers_minus1;
      uint8_t                  sps_temporal_id_nesting_flag;
      ProfileTierLevel         profile_tier_level;
      uint32_t                 sps_seq_parameter_set_id;
      uint32_t                 chroma_format_idc;
      uint8_t                  separate_colour_plane_flag;
      uint32_t                 pic_width_in_luma_samples;
      uint32_t                 pic_height_in_luma_samples;
      uint8_t                  conformance_window_flag;
      uint32_t                 conf_win_left_offset;
      uint32_t                 conf_win_right_offset;
      uint32_t                 conf_win_top_offset;
      uint32_t                 conf_win_bottom_offset;
      uint32_t                 bit_depth_luma_minus8;
      uint32_t                 bit_depth_chroma_minus8;
      uint32_t                 log2_max_pic_order_cnt_lsb_minus4;
      uint8_t                  sps_sub_layer_ordering_info_present_flag;
      std::vector<uint32_t>    sps_max_dec_pic_buffering_minus1;
      std::vector<uint32_t>    sps_max_num_reorder_pics;
      std::vector<uint32_t>    sps_max_latency_increase;
      uint32_t                 log2_min_luma_coding_block_size_minus3;
      uint32_t                 log2_diff_max_min_luma_coding_block_size;
      uint32_t                 log2_min_transform_block_size_minus2;
      uint32_t                 log2_diff_max_min_transform_block_size;
      uint32_t                 max_transform_hierarchy_depth_inter;
      uint32_t                 max_transform_hierarchy_depth_intra;
      uint8_t                  scaling_list_enabled_flag;
      uint8_t                  sps_scaling_list_data_present_flag;
      uint8_t                  amp_enabled_flag;
      uint8_t                  sample_adaptive_offset_enabled_flag;
      uint8_t                  pcm_enabled_flag;
      uint32_t                 num_short_term_ref_pic_sets;

      bool operator == (const SPS &) const;
  };

  
  class PPS: public NALUnit
  {
    public:
    PPS(): NALUnit(NAL_PPS) {};

    uint32_t     pps_pic_parameter_set_id;
    uint32_t     pps_seq_parameter_set_id;
    uint8_t      dependent_slice_segments_enabled_flag;
    uint8_t      output_flag_present_flag;
    uint8_t      num_extra_slice_header_bits;
    uint8_t      sign_data_hiding_flag;
    uint8_t      cabac_init_present_flag;
    uint32_t     num_ref_idx_l0_default_active_minus1;
    uint32_t     num_ref_idx_l1_default_active_minus1;
    int32_t      init_qp_minus26;   
    uint8_t      constrained_intra_pred_flag;
    uint8_t      transform_skip_enabled_flag;
    uint8_t      cu_qp_delta_enabled_flag;
    uint32_t     diff_cu_qp_delta_depth;
    int32_t      pps_cb_qp_offset;
    int32_t      pps_cr_qp_offset;
    uint8_t      pps_slice_chroma_qp_offsets_present_flag;
    uint8_t      weighted_pred_flag;
    uint8_t      weighted_bipred_flag;
    uint8_t      transquant_bypass_enabled_flag;
    uint8_t      tiles_enabled_flag;
    uint8_t      entropy_coding_sync_enabled_flag;

    bool operator == (const PPS &) const;
  };



  class Slice: public NALUnit
  {
    public:
      Slice(NALUnitType type): NALUnit(type) {};
      uint8_t                  first_slice_segment_in_pic_flag;
      uint8_t                  no_output_of_prior_pics_flag;
      uint32_t                 slice_pic_parameter_set_id;
      uint8_t                  dependent_slice_segment_flag;
      uint32_t                 slice_segment_address;
      std::vector<uint32_t>    slice_reserved_undetermined_flag;
      uint32_t                 slice_type;
      uint8_t                  pic_output_flag;
      uint8_t                  colour_plane_id;
      uint32_t                 pic_order_cnt_lsb;
      uint8_t                  short_term_ref_pic_set_sps_flag;
      uint8_t                  short_term_ref_pic_set_idx;
  };
}

#endif
