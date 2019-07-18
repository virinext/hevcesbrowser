#ifndef HEVC_H_
#define HEVC_H_

#include <memory>
#include <vector>
#include <array>
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

  class NALHeader
  {
  public:
    NALUnitType           type;
    uint8_t               layer_id;
    uint8_t               temporal_id_plus1;

    bool operator == (const NALHeader &) const;
  };

  class ProfileTierLevel
  {
  public:
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
    std::vector<uint8_t>   sub_layer_profile_space;
    std::vector<uint8_t>   sub_layer_tier_flag;
    std::vector<uint8_t>   sub_layer_profile_idc;
    std::vector< std::vector< uint8_t> >
                           sub_layer_profile_compatibility_flag;
    std::vector<uint8_t>   sub_layer_progressive_source_flag;
    std::vector<uint8_t>   sub_layer_interlaced_source_flag;
    std::vector<uint8_t>   sub_layer_non_packed_constraint_flag;
    std::vector<uint8_t>   sub_layer_frame_only_constraint_flag;
    std::vector<uint8_t>   sub_layer_level_idc;

    void toDefault();

    bool operator == (const ProfileTierLevel &) const;
  };

  class SubLayerHrdParameters
  {
  public:
    std::vector<uint32_t>       bit_rate_value_minus1;
    std::vector<uint32_t>       cpb_size_value_minus1;
    std::vector<uint32_t>       cpb_size_du_value_minus1;
    std::vector<uint32_t>       bit_rate_du_value_minus1;
    std::vector<uint8_t>        cbr_flag;

    void toDefault();

    bool operator == (const SubLayerHrdParameters &) const;
  };


  class ScalingListData
  {
  public:
    std::vector< std::vector< uint8_t> >    scaling_list_pred_mode_flag;
    std::vector< std::vector< uint32_t> >   scaling_list_pred_matrix_id_delta;
    std::vector< std::vector< uint32_t> >   scaling_list_dc_coef_minus8;
    std::vector<std::vector< std::vector< uint32_t> > >
                                            scaling_list_delta_coef;

    void toDefault();

    bool operator == (const ScalingListData &) const;
  };

  class HrdParameters
  {
  public:
    uint8_t               nal_hrd_parameters_present_flag;
    uint8_t               vcl_hrd_parameters_present_flag;
    uint8_t               sub_pic_hrd_params_present_flag;
    uint8_t               tick_divisor_minus2;
    uint8_t               du_cpb_removal_delay_increment_length_minus1;
    uint8_t               sub_pic_cpb_params_in_pic_timing_sei_flag;
    uint8_t               dpb_output_delay_du_length_minus1;
    uint8_t               bit_rate_scale;
    uint8_t               cpb_size_scale;
    uint8_t               cpb_size_du_scale;
    uint8_t               initial_cpb_removal_delay_length_minus1;
    uint8_t               au_cpb_removal_delay_length_minus1;
    uint8_t               dpb_output_delay_length_minus1;
    std::vector<uint8_t>  fixed_pic_rate_general_flag;
    std::vector<uint8_t>  fixed_pic_rate_within_cvs_flag;
    std::vector<uint32_t> elemental_duration_in_tc_minus1;
    std::vector<uint8_t>  low_delay_hrd_flag;
    std::vector<uint32_t> cpb_cnt_minus1;
    std::vector<SubLayerHrdParameters>
                          nal_sub_layer_hrd_parameters;
    std::vector<SubLayerHrdParameters>
                          vcl_sub_layer_hrd_parameters;

    void toDefault();

    bool operator == (const HrdParameters &) const;
 };

  class ShortTermRefPicSet
  {
  public:
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

    void toDefault();

    bool operator == (const ShortTermRefPicSet &) const;
  };

  class RefPicListModification
  {
  public:
    uint8_t                ref_pic_list_modification_flag_l0;
    std::vector<uint32_t>  list_entry_l0;
    uint8_t                ref_pic_list_modification_flag_l1;
    std::vector<uint32_t>  list_entry_l1;

    void toDefault();

    bool operator == (const RefPicListModification &) const;
  };

  class VuiParameters
  {
  public:
    uint8_t          aspect_ratio_info_present_flag;
    uint8_t          aspect_ratio_idc;
    uint16_t         sar_width;
    uint16_t         sar_height;
    uint8_t          overscan_info_present_flag;
    uint8_t          overscan_appropriate_flag;
    uint8_t          video_signal_type_present_flag;
    uint8_t          video_format;
    uint8_t          video_full_range_flag;
    uint8_t          colour_description_present_flag;
    uint8_t          colour_primaries;
    uint8_t          transfer_characteristics;
    uint8_t          matrix_coeffs;
    uint8_t          chroma_loc_info_present_flag;
    uint32_t         chroma_sample_loc_type_top_field;
    uint32_t         chroma_sample_loc_type_bottom_field;
    uint8_t          neutral_chroma_indication_flag;
    uint8_t          field_seq_flag;
    uint8_t          frame_field_info_present_flag;
    uint8_t          default_display_window_flag;
    uint32_t         def_disp_win_left_offset;
    uint32_t         def_disp_win_right_offset;
    uint32_t         def_disp_win_top_offset;
    uint32_t         def_disp_win_bottom_offset;
    uint8_t          vui_timing_info_present_flag;
    uint32_t         vui_num_units_in_tick;
    uint32_t         vui_time_scale;
    uint8_t          vui_poc_proportional_to_timing_flag;
    uint32_t         vui_num_ticks_poc_diff_one_minus1;
    uint8_t          vui_hrd_parameters_present_flag;
    HrdParameters    hrd_parameters;
    uint8_t          bitstream_restriction_flag;
    uint8_t          tiles_fixed_structure_flag;
    uint8_t          motion_vectors_over_pic_boundaries_flag;
    uint8_t          restricted_ref_pic_lists_flag;
    uint32_t         min_spatial_segmentation_idc;
    uint32_t         max_bytes_per_pic_denom;
    uint32_t         max_bits_per_min_cu_denom;
    uint32_t         log2_max_mv_length_horizontal;
    uint32_t         log2_max_mv_length_vertical;

    void toDefault();

    bool operator == (const VuiParameters &) const;

  };


  class SeiPayload
  {
  public:
    virtual ~SeiPayload() {};
  };

  class SeiMessage
  {
  public:
    enum PayloadType
    {
        BUFFERING_PERIOD                     = 0,
        PICTURE_TIMING                       = 1,
        PAN_SCAN_RECT                        = 2,
        FILLER_PAYLOAD                       = 3,
        USER_DATA_REGISTERED_ITU_T_T35       = 4,
        USER_DATA_UNREGISTERED               = 5,
        RECOVERY_POINT                       = 6,
        SCENE_INFO                           = 9,
        FULL_FRAME_SNAPSHOT                  = 15,
        PROGRESSIVE_REFINEMENT_SEGMENT_START = 16,
        PROGRESSIVE_REFINEMENT_SEGMENT_END   = 17,
        FILM_GRAIN_CHARACTERISTICS           = 19,
        POST_FILTER_HINT                     = 22,
        TONE_MAPPING_INFO                    = 23,
        FRAME_PACKING                        = 45,
        DISPLAY_ORIENTATION                  = 47,
        SOP_DESCRIPTION                      = 128,
        ACTIVE_PARAMETER_SETS                = 129,
        DECODING_UNIT_INFO                   = 130,
        TEMPORAL_LEVEL0_INDEX                = 131,
        DECODED_PICTURE_HASH                 = 132,
        SCALABLE_NESTING                     = 133,
        REGION_REFRESH_INFO                  = 134,
        NO_DISPLAY                           = 135,
        TIME_CODE                            = 136,
        MASTERING_DISPLAY_INFO               = 137,
        SEGM_RECT_FRAME_PACKING              = 138,
        TEMP_MOTION_CONSTRAINED_TILE_SETS    = 139,
        CHROMA_RESAMPLING_FILTER_HINT        = 140,
        KNEE_FUNCTION_INFO                   = 141,
        COLOUR_REMAPPING_INFO                = 142,
        CONTENT_LIGHT_LEVEL_INFO             = 144,
        ALTERNATIVE_TRANSFER_CHARACTERISTICS = 147,
    };

    uint32_t           num_payload_type_ff_bytes;
    uint32_t           num_payload_size_ff_bytes;
    uint8_t            last_payload_type_byte;
    uint8_t            last_payload_size_byte;
    std::shared_ptr<SeiPayload>
                       sei_payload;

    virtual void toDefault();
  };


  class DecodedPictureHash: public SeiPayload
  {
  public:
    uint8_t                                   hash_type;
    std::vector<std::array<uint8_t, 16> >     picture_md5;
    std::vector<uint16_t>                     picture_crc;
    std::vector<uint32_t>                     picture_checksum;

    void toDefault();
  };

  class UserDataUnregistered: public SeiPayload
  {
  public:
    uint8_t                               uuid_iso_iec_11578[16];
    std::vector<uint8_t>                  user_data_payload_byte;

    void toDefault() {};
  };

  class SceneInfo: public SeiPayload
  {
  public:
    uint8_t                               scene_info_present_flag;
    uint8_t                               prev_scene_id_valid_flag;
    uint32_t                              scene_id;
    uint32_t                              scene_transition_type;
    uint32_t                              second_scene_id;

    void toDefault() {};
  };

  class FullFrameSnapshot: public SeiPayload
  {
  public:
    uint32_t                              snapshot_id;

    void toDefault() {};
  };

  class ProgressiveRefinementSegmentStart: public SeiPayload
  {
  public:
    uint32_t                              progressive_refinement_id;
    uint32_t                              pic_order_cnt_delta;

    void toDefault() {};
  };

  class ProgressiveRefinementSegmentEnd: public SeiPayload
  {
  public:
    uint32_t                              progressive_refinement_id;

    void toDefault() {};
  };

  class BufferingPeriod: public SeiPayload
  {
  public:
    uint32_t                           bp_seq_parameter_set_id;
    uint8_t                            irap_cpb_params_present_flag;
    uint32_t                           cpb_delay_offset;
    uint32_t                           dpb_delay_offset;
    uint8_t                            concatenation_flag;
    uint32_t                           au_cpb_removal_delay_delta_minus1;
    std::vector<uint32_t>              nal_initial_cpb_removal_delay;
    std::vector<uint32_t>              nal_initial_cpb_removal_offset;
    std::vector<uint32_t>              nal_initial_alt_cpb_removal_delay;
    std::vector<uint32_t>              nal_initial_alt_cpb_removal_offset;
    std::vector<uint32_t>              vcl_initial_cpb_removal_delay;
    std::vector<uint32_t>              vcl_initial_cpb_removal_offset;
    std::vector<uint32_t>              vcl_initial_alt_cpb_removal_delay;
    std::vector<uint32_t>              vcl_initial_alt_cpb_removal_offset;

    void toDefault();
  };

  class FillerPayload: public SeiPayload
  {
  public:
    void toDefault() {};
  };

  class PicTiming: public SeiPayload
  {
  public:
    uint8_t                            pic_struct;
    uint8_t                            source_scan_type;
    uint8_t                            duplicate_flag;
    uint32_t                           au_cpb_removal_delay_minus1;
    uint32_t                           pic_dpb_output_delay;
    uint32_t                           pic_dpb_output_du_delay;
    uint32_t                           num_decoding_units_minus1;
    uint8_t                            du_common_cpb_removal_delay_flag;
    uint32_t                           du_common_cpb_removal_delay_increment_minus1;
    std::vector<uint32_t>              num_nalus_in_du_minus1;
    std::vector<uint32_t>              du_cpb_removal_delay_increment_minus1;

    void toDefault();
  };

  class RecoveryPoint: public SeiPayload
  {
  public:
    uint32_t                           recovery_poc_cnt;
    uint8_t                            exact_match_flag;
    uint8_t                            broken_link_flag;
    void toDefault() {};
  };


  class ActiveParameterSets: public SeiPayload
  {
  public:
    uint8_t                 active_video_parameter_set_id;
    uint8_t                 self_contained_cvs_flag;
    uint8_t                 no_parameter_set_update_flag;
    uint32_t                num_sps_ids_minus1;
    std::vector<uint32_t>   active_seq_parameter_set_id;

    void toDefault() {};
  };


  class TemporalLevel0Index: public SeiPayload
  {
  public:
    uint8_t                 temporal_sub_layer_zero_idx;
    uint8_t                 irap_pic_id;

    void toDefault() {};
  };

  class RegionRefreshInfo: public SeiPayload
  {
  public:
    uint8_t                 refreshed_region_flag;

    void toDefault() {};
  };

  class ToneMapping: public SeiPayload
  {
  public:
    uint32_t                tone_map_id;
    uint8_t                 tone_map_cancel_flag;
    uint8_t                 tone_map_persistence_flag;
    uint8_t                 coded_data_bit_depth;
    uint8_t                 target_bit_depth;
    uint32_t                tone_map_model_id;
    uint32_t                min_value;
    uint32_t                max_value;
    uint32_t                sigmoid_midpoint;
    uint32_t                sigmoid_width;
    std::vector<uint32_t>   start_of_coded_interval;
    uint16_t                num_pivots;
    std::vector<uint32_t>   coded_pivot_value;
    std::vector<uint32_t>   target_pivot_value;
    uint8_t                 camera_iso_speed_idc;
    uint32_t                camera_iso_speed_value;
    uint8_t                 exposure_index_idc;
    uint32_t                exposure_index_value;
    uint8_t                 exposure_compensation_value_sign_flag;
    uint16_t                exposure_compensation_value_numerator;
    uint16_t                exposure_compensation_value_denom_idc;
    uint32_t                ref_screen_luminance_white;
    uint32_t                extended_range_white_level;
    uint16_t                nominal_black_level_code_value;
    uint16_t                nominal_white_level_code_value;
    uint16_t                extended_white_level_code_value;

    void toDefault() {};
  };


  class FramePacking: public SeiPayload
  {
  public:
    uint32_t                frame_packing_arrangement_id;
    uint8_t                 frame_packing_arrangement_cancel_flag;
    uint8_t                 frame_packing_arrangement_type;
    uint8_t                 quincunx_sampling_flag;
    uint8_t                 content_interpretation_type;
    uint8_t                 spatial_flipping_flag;
    uint8_t                 frame0_flipped_flag;
    uint8_t                 field_views_flag;
    uint8_t                 current_frame_is_frame0_flag;
    uint8_t                 frame0_self_contained_flag;
    uint8_t                 frame1_self_contained_flag;
    uint8_t                 frame0_grid_position_x;
    uint8_t                 frame0_grid_position_y;
    uint8_t                 frame1_grid_position_x;
    uint8_t                 frame1_grid_position_y;
    uint8_t                 frame_packing_arrangement_reserved_byte;
    uint8_t                 frame_packing_arrangement_persistence_flag;
    uint8_t                 upsampled_aspect_ratio_flag;

    void toDefault() {};
  };


  class DisplayOrientation: public SeiPayload
  {
  public:
    uint8_t                 display_orientation_cancel_flag;
    uint8_t                 hor_flip;
    uint8_t                 ver_flip;
    uint16_t                anticlockwise_rotation;
    uint8_t                 display_orientation_persistence_flag;

    void toDefault() {};
  };

  class SOPDescription: public SeiPayload
  {
  public:
    uint32_t                sop_seq_parameter_set_id;
    uint32_t                num_entries_in_sop_minus1;
    std::vector<uint8_t>    sop_vcl_nut;
    std::vector<uint8_t>    sop_temporal_id;
    std::vector<uint32_t>   sop_short_term_rps_idx;
    std::vector<uint32_t>   sop_poc_delta;

    void toDefault() {};
  };

  class TimeCode: public SeiPayload
  {
  public:
    uint8_t                       num_clock_ts;
    std::vector<uint8_t>          clock_time_stamp_flag;
    std::vector<uint8_t>          nuit_field_based_flag;
    std::vector<uint8_t>          counting_type;
    std::vector<uint8_t>          full_timestamp_flag;
    std::vector<uint8_t>          discontinuity_flag;
    std::vector<uint8_t>          cnt_dropped_flag;
    std::vector<uint16_t>         n_frames;
    std::vector<uint8_t>          seconds_value;
    std::vector<uint8_t>          minutes_value;
    std::vector<uint8_t>          hours_value;
    std::vector<uint8_t>          seconds_flag;
    std::vector<uint8_t>          minutes_flag;
    std::vector<uint8_t>          hours_flag;
    std::vector<uint8_t>          time_offset_length;
    std::vector<uint32_t>         time_offset_value;

    void toDefault() {};
  };

  class MasteringDisplayInfo: public SeiPayload
  {
  public:
    uint16_t      display_primary_x[3];
    uint16_t      display_primary_y[3];
    uint16_t      white_point_x;
    uint16_t      white_point_y;
    uint32_t      max_display_mastering_luminance;
    uint32_t      min_display_mastering_luminance;

    void toDefault();
  };

  class SegmRectFramePacking: public SeiPayload
  {
  public:
    uint8_t      segmented_rect_frame_packing_arrangement_cancel_flag;
    uint8_t      segmented_rect_content_interpretation_type;
    uint8_t      segmented_rect_frame_packing_arrangement_persistence;

    void toDefault();
  };

  class KneeFunctionInfo: public SeiPayload
  {
  public:
    uint32_t                  knee_function_id;
    uint8_t                   knee_function_cancel_flag;
    uint8_t                   knee_function_persistence_flag;
    uint32_t                  input_d_range;
    uint32_t                  input_disp_luminance;
    uint32_t                  output_d_range;
    uint32_t                  output_disp_luminance;
    uint32_t                  num_knee_points_minus1;
    std::vector<uint16_t>     input_knee_point;
    std::vector<uint16_t>     output_knee_point;

    void toDefault();
  };

  class ChromaResamplingFilterHint: public SeiPayload
  {
  public:
    uint8_t                   ver_chroma_filter_idc;
    uint8_t                   hor_chroma_filter_idc;
    uint8_t                   ver_filtering_field_processing_flag;
    uint32_t                  target_format_idc;
    uint32_t                  num_vertical_filters;
    std::vector<uint32_t>     ver_tap_length_minus_1;
    std::vector<
     std::vector<int32_t> >   ver_filter_coeff;

    uint32_t                  num_horizontal_filters;
    std::vector<uint32_t>     hor_tap_length_minus_1;
    std::vector<
     std::vector<int32_t> >   hor_filter_coeff;

    void toDefault();
  };

  class ColourRemappingInfo: public SeiPayload
  {
  public:
    uint32_t                     colour_remap_id;
    uint8_t                      colour_remap_cancel_flag;
    uint8_t                      colour_remap_persistence_flag;
    uint8_t                      colour_remap_video_signal_info_present_flag;
    uint8_t                      colour_remap_full_range_flag;
    uint8_t                      colour_remap_primaries;
    uint8_t                      colour_remap_transfer_function;
    uint8_t                      colour_remap_matrix_coefficients;
    uint8_t                      colour_remap_input_bit_depth;
    uint8_t                      colour_remap_bit_depth;
    uint8_t                      pre_lut_num_val_minus1[3];
    std::vector<uint32_t>        pre_lut_coded_value[3];
    std::vector<uint32_t>        pre_lut_target_value[3];
    uint8_t                      colour_remap_matrix_present_flag;
    uint8_t                      log2_matrix_denom;
    int32_t                      colour_remap_coeffs[3][3];
    uint8_t                      post_lut_num_val_minus1[3];
    std::vector<uint32_t>        post_lut_coded_value[3];
    std::vector<uint32_t>        post_lut_target_value[3];

    void toDefault() {};
  };

  class ContentLightLevelInfo: public SeiPayload
  {
  public:
    uint16_t      max_content_light_level;
    uint16_t      max_pic_average_light_level;

    void toDefault() {};
  };

  class AlternativeTransferCharacteristics: public SeiPayload
  {
  public:
    uint16_t      alternative_transfer_characteristics;

    void toDefault() {};
  };

  class PredWeightTable
  {
  public:
    uint32_t                  luma_log2_weight_denom;
    int32_t                   delta_chroma_log2_weight_denom;
    std::vector<uint8_t>      luma_weight_l0_flag;
    std::vector<uint8_t>      chroma_weight_l0_flag;
    std::vector<int32_t>      delta_luma_weight_l0;
    std::vector<int32_t>      luma_offset_l0;
    std::vector<std::array<int32_t, 2> >
                              delta_chroma_weight_l0;
    std::vector<std::array<int32_t, 2> >
                              delta_chroma_offset_l0;
    std::vector<uint8_t>      luma_weight_l1_flag;
    std::vector<uint8_t>      chroma_weight_l1_flag;
    std::vector<int32_t>      delta_luma_weight_l1;
    std::vector<int32_t>      luma_offset_l1;
    std::vector<std::array<int32_t, 2> >
                              delta_chroma_weight_l1;
    std::vector<std::array<int32_t, 2> >
                              delta_chroma_offset_l1;

    void toDefault();
  };

  class NALUnit
  {
    public:
      NALUnit(NALHeader header);
      virtual ~NALUnit();
      virtual NALUnitType getType() const;

      std::shared_ptr<NALUnit> copy() const;

      bool            m_processFailed;

      NALHeader     m_nalHeader;
  };



  class VPS: public NALUnit
  {
    public:
      VPS();
      uint8_t                   vps_video_parameter_set_id;
      uint8_t                   vps_max_layers_minus1;
      uint8_t                   vps_max_sub_layers_minus1;
      uint8_t                   vps_temporal_id_nesting_flag;
      ProfileTierLevel          profile_tier_level;
      uint8_t                   vps_sub_layer_ordering_info_present_flag;
      std::vector<uint32_t>     vps_max_dec_pic_buffering_minus1;
      std::vector<uint32_t>     vps_max_num_reorder_pics;
      std::vector<uint32_t>     vps_max_latency_increase_plus1;
      uint8_t                   vps_max_layer_id;
      uint32_t                  vps_num_layer_sets_minus1;
      std::vector<std::vector<uint8_t> >
                                layer_id_included_flag;
      uint8_t                   vps_timing_info_present_flag;
      uint32_t                  vps_num_units_in_tick;
      uint32_t                  vps_time_scale;
      uint8_t                   vps_poc_proportional_to_timing_flag;
      uint32_t                  vps_num_ticks_poc_diff_one_minus1;
      uint32_t                  vps_num_hrd_parameters;
      std::vector<uint32_t>     hrd_layer_set_idx;
      std::vector<uint8_t>      cprms_present_flag;
      std::vector<HrdParameters>
                                hrd_parameters;
      uint8_t                   vps_extension_flag;

      void toDefault();
      bool operator == (const VPS &) const;
  };


  class SPS: public NALUnit
  {
    public:
      SPS();
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
      std::vector<uint32_t>    sps_max_latency_increase_plus1;
      uint32_t                 log2_min_luma_coding_block_size_minus3;
      uint32_t                 log2_diff_max_min_luma_coding_block_size;
      uint32_t                 log2_min_transform_block_size_minus2;
      uint32_t                 log2_diff_max_min_transform_block_size;
      uint32_t                 max_transform_hierarchy_depth_inter;
      uint32_t                 max_transform_hierarchy_depth_intra;
      uint8_t                  scaling_list_enabled_flag;
      uint8_t                  sps_scaling_list_data_present_flag;
      ScalingListData          scaling_list_data;
      uint8_t                  amp_enabled_flag;
      uint8_t                  sample_adaptive_offset_enabled_flag;
      uint8_t                  pcm_enabled_flag;
      uint8_t                  pcm_sample_bit_depth_luma_minus1;
      uint8_t                  pcm_sample_bit_depth_chroma_minus1;
      uint32_t                 log2_min_pcm_luma_coding_block_size_minus3;
      uint32_t                 log2_diff_max_min_pcm_luma_coding_block_size;
      uint8_t                  pcm_loop_filter_disabled_flag;
      uint32_t                 num_short_term_ref_pic_sets;
      std::vector<ShortTermRefPicSet>
                               short_term_ref_pic_set;
      uint8_t                  long_term_ref_pics_present_flag;
      uint32_t                 num_long_term_ref_pics_sps;
      std::vector<uint32_t>    lt_ref_pic_poc_lsb_sps;
      std::vector<uint8_t>     used_by_curr_pic_lt_sps_flag;
      uint8_t                  sps_temporal_mvp_enabled_flag;
      uint8_t                  strong_intra_smoothing_enabled_flag;
      uint8_t                  vui_parameters_present_flag;
      VuiParameters            vui_parameters;
      uint8_t                  sps_extension_flag;

      void toDefault();

      bool operator == (const SPS &) const;
  };


  class PPS: public NALUnit
  {
    public:
    PPS();

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
    uint32_t     num_tile_columns_minus1;
    uint32_t     num_tile_rows_minus1;
    uint8_t      uniform_spacing_flag;
    std::vector<uint32_t>
                 column_width_minus1;
    std::vector<uint32_t>
                 row_height_minus1;
    uint8_t      loop_filter_across_tiles_enabled_flag;
    uint8_t      pps_loop_filter_across_slices_enabled_flag;
    uint8_t      deblocking_filter_control_present_flag;
    uint8_t      deblocking_filter_override_enabled_flag;
    uint8_t      pps_deblocking_filter_disabled_flag;
    uint32_t     pps_beta_offset_div2;
    uint32_t     pps_tc_offset_div2;
    uint8_t      pps_scaling_list_data_present_flag;
    ScalingListData
                 scaling_list_data;
    uint8_t      lists_modification_present_flag;
    int32_t      log2_parallel_merge_level_minus2;
    uint8_t      slice_segment_header_extension_present_flag;
    uint8_t      pps_extension_flag;

    void toDefault();

    bool operator == (const PPS &) const;
  };



  class Slice: public NALUnit
  {
    public:
      enum SliceType
      {
        B_SLICE = 0,
        P_SLICE = 1,
        I_SLICE = 2,
        NONE_SLICE = 3
      };

      Slice(NALHeader header);
      uint8_t                  first_slice_segment_in_pic_flag;
      uint8_t                  no_output_of_prior_pics_flag;
      uint32_t                 slice_pic_parameter_set_id;
      uint8_t                  dependent_slice_segment_flag;
      uint32_t                 slice_segment_address;
      std::vector<uint32_t>    slice_reserved_undetermined_flag;
      uint32_t                 slice_type;
      uint8_t                  pic_output_flag;
      uint8_t                  colour_plane_id;
      uint32_t                 slice_pic_order_cnt_lsb;
      uint8_t                  short_term_ref_pic_set_sps_flag;
      ShortTermRefPicSet       short_term_ref_pic_set;
      uint8_t                  short_term_ref_pic_set_idx;

      uint32_t                 num_long_term_sps;
      uint32_t                 num_long_term_pics;
      std::vector<uint32_t>    lt_idx_sps;
      std::vector<uint32_t>    poc_lsb_lt;
      std::vector<uint8_t>     used_by_curr_pic_lt_flag;
      std::vector<uint8_t>     delta_poc_msb_present_flag;
      std::vector<uint32_t>    delta_poc_msb_cycle_lt;

      uint8_t                  slice_temporal_mvp_enabled_flag;
      uint8_t                  slice_sao_luma_flag;
      uint8_t                  slice_sao_chroma_flag;
      uint8_t                  num_ref_idx_active_override_flag;
      uint32_t                 num_ref_idx_l0_active_minus1;
      uint32_t                 num_ref_idx_l1_active_minus1;
      RefPicListModification   ref_pic_lists_modification;
      uint8_t                  mvd_l1_zero_flag;
      uint8_t                  cabac_init_flag;
      uint8_t                  collocated_from_l0_flag;
      uint32_t                 collocated_ref_idx;
      PredWeightTable          pred_weight_table;
      uint32_t                 five_minus_max_num_merge_cand;
      int32_t                  slice_qp_delta;
      int32_t                  slice_cb_qp_offset;
      int32_t                  slice_cr_qp_offset;
      uint8_t                  deblocking_filter_override_flag;
      uint8_t                  slice_deblocking_filter_disabled_flag;
      int32_t                  slice_beta_offset_div2;
      int32_t                  slice_tc_offset_div2;
      int32_t                  slice_loop_filter_across_slices_enabled_flag;
      uint32_t                 num_entry_point_offsets;
      uint32_t                 offset_len_minus1;
      std::vector<uint32_t>    entry_point_offset_minus1;
      uint32_t                 slice_segment_header_extension_length;
      std::vector<uint8_t>     slice_segment_header_extension_data_byte;


      void toDefault();
  };

  class AUD: public NALUnit
  {
  public:
    AUD();

    uint8_t            pic_type;
    void toDefault();
  };


  class SEI: public NALUnit
  {
  public:
    SEI(NALHeader header);
    std::vector<SeiMessage>     sei_message;

    void toDefault();
  };

}

#endif
