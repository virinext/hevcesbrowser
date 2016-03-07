#include <boost/test/unit_test.hpp>

#include "Params.h"
#include "Hevc.h"
#include "HevcParser.h"

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace HEVC;

BOOST_AUTO_TEST_SUITE(parsing);


class Consumer: public Parser::Consumer
{
  public:
    struct NALUInfo
    {
      std::shared_ptr<NALUnit>      m_pnalu;
      Parser::Info                  m_info;
    };
    
    virtual void onNALUnit(std::shared_ptr<NALUnit> pNALUnit, const Parser::Info *pInfo)
    {
      NALUInfo nalu;
      nalu.m_pnalu = pNALUnit;
      nalu.m_info = *pInfo;
            
      m_nalus.push_back(nalu);
    }

    virtual void onWarning(const std::string &warning, const Parser::Info *pInfo, Parser::WarningType type) {};
        
    std::vector<NALUInfo>    m_nalus;
};

BOOST_AUTO_TEST_CASE(SURFING_FIRST_30)
{
  Parser *pparser = Parser::create();
  
  Consumer consumer;
  
  pparser -> addConsumer(&consumer);
  
  std::ifstream in(getSourceDir() + "/samples/surfing_30.265", std::ios::binary);
  
  in.seekg(0, std::ios::end);
  std::size_t size = in.tellg();
  in.seekg(0, std::ios::beg);
  
  char *pdata = new char[size];
  in.read(pdata, size);
  size = in.gcount();
  pparser -> process((const uint8_t *)pdata, size);

  pparser -> releaseConsumer(&consumer);

  Parser::release(pparser);
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_info.m_position, 0);
  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_pnalu -> getType(), NAL_VPS);

  std::shared_ptr<VPS> pvps = std::static_pointer_cast<VPS>(consumer.m_nalus[0].m_pnalu);
  
  BOOST_CHECK_EQUAL(pvps -> vps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_temporal_id_nesting_flag, 1);
  BOOST_CHECK_EQUAL(pvps -> vps_timing_info_present_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_extension_flag, 0);

  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_info.m_position, 0x1d);
  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_pnalu -> getType(), NAL_SPS);

  std::shared_ptr<SPS> psps = std::static_pointer_cast<SPS>(consumer.m_nalus[1].m_pnalu);
  BOOST_CHECK_EQUAL(psps -> sps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(psps -> sps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_id_nesting_flag, 1);

  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_profile_space, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_tier_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_profile_idc, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_progressive_source_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_interlaced_source_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_non_packed_constraint_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_frame_only_constraint_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_level_idc, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.sub_layer_profile_present_flag.size(), 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.sub_layer_level_present_flag.size(), 0);

  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_profile_compatibility_flag[0], 1);

  BOOST_CHECK_EQUAL(psps -> sps_seq_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(psps -> chroma_format_idc, 1);

  BOOST_CHECK_EQUAL(psps -> pic_width_in_luma_samples, 1280);
  BOOST_CHECK_EQUAL(psps -> pic_height_in_luma_samples, 720);
  BOOST_CHECK_EQUAL(psps -> conformance_window_flag, 1);
  BOOST_CHECK_EQUAL(psps -> conf_win_left_offset, 0);
  BOOST_CHECK_EQUAL(psps -> conf_win_right_offset, 0);
  BOOST_CHECK_EQUAL(psps -> conf_win_top_offset, 0);
  BOOST_CHECK_EQUAL(psps -> conf_win_bottom_offset, 0);
  BOOST_CHECK_EQUAL(psps -> bit_depth_luma_minus8, 0);
  BOOST_CHECK_EQUAL(psps -> bit_depth_chroma_minus8, 0);
  BOOST_CHECK_EQUAL(psps -> log2_max_pic_order_cnt_lsb_minus4, 4);
  BOOST_CHECK_EQUAL(psps -> sps_sub_layer_ordering_info_present_flag, 1);

  BOOST_CHECK_EQUAL(psps -> sps_max_dec_pic_buffering_minus1[0], 2);
  BOOST_CHECK_EQUAL(psps -> sps_max_num_reorder_pics[0], 1);
  BOOST_CHECK_EQUAL(psps -> sps_max_latency_increase_plus1[0], 0);

  BOOST_CHECK_EQUAL(psps -> log2_min_luma_coding_block_size_minus3, 0);
  BOOST_CHECK_EQUAL(psps -> log2_diff_max_min_luma_coding_block_size, 3);
  BOOST_CHECK_EQUAL(psps -> log2_min_transform_block_size_minus2, 0);
  BOOST_CHECK_EQUAL(psps -> log2_diff_max_min_transform_block_size, 3);
  BOOST_CHECK_EQUAL(psps -> max_transform_hierarchy_depth_inter, 2);
  BOOST_CHECK_EQUAL(psps -> max_transform_hierarchy_depth_intra, 2);
  
  BOOST_CHECK_EQUAL(psps -> scaling_list_enabled_flag, 0);
  BOOST_CHECK_EQUAL(psps -> amp_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> sample_adaptive_offset_enabled_flag, 0);
  BOOST_CHECK_EQUAL(psps -> pcm_enabled_flag, 0);
  BOOST_CHECK_EQUAL(psps -> num_short_term_ref_pic_sets, 5);

  BOOST_CHECK_EQUAL(psps -> long_term_ref_pics_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_mvp_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> strong_intra_smoothing_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sps_extension_flag, 0);

  
  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_info.m_position, 0x49);
  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_pnalu -> getType(), NAL_PPS);

  std::shared_ptr<PPS> ppps = std::static_pointer_cast<PPS>(consumer.m_nalus[2].m_pnalu);
  BOOST_CHECK_EQUAL(ppps -> pps_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_seq_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> dependent_slice_segments_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> output_flag_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> num_extra_slice_header_bits, 0);
  BOOST_CHECK_EQUAL(ppps -> sign_data_hiding_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> cabac_init_present_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> num_ref_idx_l0_default_active_minus1, 1);
  BOOST_CHECK_EQUAL(ppps -> num_ref_idx_l1_default_active_minus1, 1);
  BOOST_CHECK_EQUAL(ppps -> init_qp_minus26, 0);
  BOOST_CHECK_EQUAL(ppps -> constrained_intra_pred_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> transform_skip_enabled_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> cu_qp_delta_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_cb_qp_offset, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_cr_qp_offset, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_slice_chroma_qp_offsets_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> weighted_pred_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> weighted_bipred_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> transquant_bypass_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> tiles_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> entropy_coding_sync_enabled_flag, 0);
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_info.m_position, 0x54);
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_pnalu -> getType(), NAL_IDR_W_RADL);
  std::shared_ptr<Slice> pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[3].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> no_output_of_prior_pics_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> dependent_slice_segment_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_reserved_undetermined_flag.size(), 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 2);
  
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_info.m_position, 0x6a3d);
  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_pnalu -> getType(), NAL_TRAIL_R);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[4].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> dependent_slice_segment_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_reserved_undetermined_flag.size(), 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 1);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 4);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 1);
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_info.m_position, 0xa09b);
  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_pnalu -> getType(), NAL_TRAIL_N);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[5].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> dependent_slice_segment_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_reserved_undetermined_flag.size(), 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 0);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 1);

  
  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_info.m_position, 0xac6a);
  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_pnalu -> getType(), NAL_TRAIL_N);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[6].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> dependent_slice_segment_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_reserved_undetermined_flag.size(), 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 0);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 2);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 1);
  

  BOOST_CHECK_EQUAL(consumer.m_nalus[7].m_info.m_position, 0xbb8d);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[7].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> dependent_slice_segment_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_reserved_undetermined_flag.size(), 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 0);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 3);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 1);

  BOOST_CHECK_EQUAL(consumer.m_nalus[7].m_pnalu -> getType(), NAL_TRAIL_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[8].m_info.m_position, 0xc8f0);
  BOOST_CHECK_EQUAL(consumer.m_nalus[8].m_pnalu -> getType(), NAL_TRAIL_R);
  BOOST_CHECK_EQUAL(consumer.m_nalus[9].m_info.m_position, 0x10782);
  BOOST_CHECK_EQUAL(consumer.m_nalus[9].m_pnalu -> getType(), NAL_TRAIL_N);



  BOOST_CHECK_EQUAL(consumer.m_nalus[10].m_info.m_position, 0x11691);
  BOOST_CHECK_EQUAL(consumer.m_nalus[10].m_pnalu -> getType(), NAL_TRAIL_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[11].m_info.m_position, 0x000128CD);
  BOOST_CHECK_EQUAL(consumer.m_nalus[11].m_pnalu -> getType(), NAL_TRAIL_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[12].m_info.m_position, 0x000137EA);
  BOOST_CHECK_EQUAL(consumer.m_nalus[12].m_pnalu -> getType(), NAL_TRAIL_R);
  BOOST_CHECK_EQUAL(consumer.m_nalus[13].m_info.m_position, 0x00017301);
  BOOST_CHECK_EQUAL(consumer.m_nalus[13].m_pnalu -> getType(), NAL_TRAIL_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[14].m_info.m_position, 0x00018348);
  BOOST_CHECK_EQUAL(consumer.m_nalus[14].m_pnalu -> getType(), NAL_TRAIL_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[15].m_info.m_position, 0x00019643);
  BOOST_CHECK_EQUAL(consumer.m_nalus[15].m_pnalu -> getType(), NAL_TRAIL_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[16].m_info.m_position, 0x0001A6B0);
  BOOST_CHECK_EQUAL(consumer.m_nalus[16].m_pnalu -> getType(), NAL_TRAIL_R);
  BOOST_CHECK_EQUAL(consumer.m_nalus[17].m_info.m_position, 0x0001E00A);
  BOOST_CHECK_EQUAL(consumer.m_nalus[17].m_pnalu -> getType(), NAL_TRAIL_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[18].m_info.m_position, 0x0001EE99);
  BOOST_CHECK_EQUAL(consumer.m_nalus[18].m_pnalu -> getType(), NAL_TRAIL_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[19].m_info.m_position, 0x0001FF61);
  BOOST_CHECK_EQUAL(consumer.m_nalus[19].m_pnalu -> getType(), NAL_TRAIL_N);


  BOOST_CHECK_EQUAL(consumer.m_nalus[20].m_info.m_position, 0x00020CDB);
  BOOST_CHECK_EQUAL(consumer.m_nalus[20].m_pnalu -> getType(), NAL_TRAIL_R);
  BOOST_CHECK_EQUAL(consumer.m_nalus[21].m_info.m_position, 0x00024F57);
  BOOST_CHECK_EQUAL(consumer.m_nalus[21].m_pnalu -> getType(), NAL_TRAIL_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[22].m_info.m_position, 0x00025C81);
  BOOST_CHECK_EQUAL(consumer.m_nalus[22].m_pnalu -> getType(), NAL_TRAIL_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[23].m_info.m_position, 0x00026C34);
  BOOST_CHECK_EQUAL(consumer.m_nalus[23].m_pnalu -> getType(), NAL_TRAIL_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[24].m_info.m_position, 0x00027A22);
  BOOST_CHECK_EQUAL(consumer.m_nalus[24].m_pnalu -> getType(), NAL_TRAIL_R);
  BOOST_CHECK_EQUAL(consumer.m_nalus[25].m_info.m_position, 0x0002B78D);
  BOOST_CHECK_EQUAL(consumer.m_nalus[25].m_pnalu -> getType(), NAL_TRAIL_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[26].m_info.m_position, 0x0002C535);
  BOOST_CHECK_EQUAL(consumer.m_nalus[26].m_pnalu -> getType(), NAL_TRAIL_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[27].m_info.m_position, 0x0002D588);
  BOOST_CHECK_EQUAL(consumer.m_nalus[27].m_pnalu -> getType(), NAL_TRAIL_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[28].m_info.m_position, 0x0002E367);
  BOOST_CHECK_EQUAL(consumer.m_nalus[28].m_pnalu -> getType(), NAL_TRAIL_R);
  BOOST_CHECK_EQUAL(consumer.m_nalus[29].m_info.m_position, 0x00031D2E);
  BOOST_CHECK_EQUAL(consumer.m_nalus[29].m_pnalu -> getType(), NAL_TRAIL_N);

}


BOOST_AUTO_TEST_CASE(SINTEL_FIRST_30)
{
  Parser *pparser = Parser::create();
  
  Consumer consumer;
  
  pparser -> addConsumer(&consumer);
  
  std::ifstream in(getSourceDir() + "/samples/Sintel_272p_logo_30.265", std::ios::binary);
  
  in.seekg(0, std::ios::end);
  std::size_t size = in.tellg();
  in.seekg(0, std::ios::beg);
  
  char *pdata = new char[size];
  in.read(pdata, size);
  size = in.gcount();
  pparser -> process((const uint8_t *)pdata, size);
    
  pparser -> releaseConsumer(&consumer);
  Parser::release(pparser);
  

  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_info.m_position, 0);
  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_pnalu -> getType(), NAL_VPS);

  std::shared_ptr<VPS> pvps = std::static_pointer_cast<VPS>(consumer.m_nalus[0].m_pnalu);
  
  BOOST_CHECK_EQUAL(pvps -> vps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_sub_layers_minus1, 2);
  BOOST_CHECK_EQUAL(pvps -> vps_temporal_id_nesting_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_timing_info_present_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_extension_flag, 0);
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_info.m_position, 0x00000021);
  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_pnalu -> getType(), NAL_SPS);

  std::shared_ptr<SPS> psps = std::static_pointer_cast<SPS>(consumer.m_nalus[1].m_pnalu);
  BOOST_CHECK_EQUAL(psps -> sps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(psps -> sps_max_sub_layers_minus1, 2);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_id_nesting_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_profile_space, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_tier_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_profile_idc, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_progressive_source_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_interlaced_source_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_non_packed_constraint_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_frame_only_constraint_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_level_idc, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.sub_layer_profile_present_flag.size(), 2);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.sub_layer_level_present_flag.size(), 2);

  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_profile_compatibility_flag[0], 1);

  BOOST_CHECK_EQUAL(psps -> profile_tier_level.sub_layer_profile_present_flag[0], 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.sub_layer_profile_present_flag[1], 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.sub_layer_level_present_flag[0], 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.sub_layer_level_present_flag[1], 0);

  BOOST_CHECK_EQUAL(psps -> sps_seq_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(psps -> chroma_format_idc, 1);

  BOOST_CHECK_EQUAL(psps -> pic_width_in_luma_samples, 640);
  BOOST_CHECK_EQUAL(psps -> pic_height_in_luma_samples, 272);
  BOOST_CHECK_EQUAL(psps -> conformance_window_flag, 1);
  BOOST_CHECK_EQUAL(psps -> conf_win_left_offset, 0);
  BOOST_CHECK_EQUAL(psps -> conf_win_right_offset, 0);
  BOOST_CHECK_EQUAL(psps -> conf_win_top_offset, 0);
  BOOST_CHECK_EQUAL(psps -> conf_win_bottom_offset, 0);
  BOOST_CHECK_EQUAL(psps -> bit_depth_luma_minus8, 0);
  BOOST_CHECK_EQUAL(psps -> bit_depth_chroma_minus8, 0);
  BOOST_CHECK_EQUAL(psps -> log2_max_pic_order_cnt_lsb_minus4, 4);
  BOOST_CHECK_EQUAL(psps -> sps_sub_layer_ordering_info_present_flag, 1);

  BOOST_CHECK_EQUAL(psps -> sps_max_dec_pic_buffering_minus1[0], 2);
  BOOST_CHECK_EQUAL(psps -> sps_max_num_reorder_pics[0], 0);
  BOOST_CHECK_EQUAL(psps -> sps_max_latency_increase_plus1[0], 0);
  BOOST_CHECK_EQUAL(psps -> sps_max_dec_pic_buffering_minus1[1], 2);
  BOOST_CHECK_EQUAL(psps -> sps_max_num_reorder_pics[1], 1);
  BOOST_CHECK_EQUAL(psps -> sps_max_latency_increase_plus1[1], 0);
  BOOST_CHECK_EQUAL(psps -> sps_max_dec_pic_buffering_minus1[2], 3);
  BOOST_CHECK_EQUAL(psps -> sps_max_num_reorder_pics[2], 2);
  BOOST_CHECK_EQUAL(psps -> sps_max_latency_increase_plus1[2], 0);

  BOOST_CHECK_EQUAL(psps -> log2_min_luma_coding_block_size_minus3, 0);
  BOOST_CHECK_EQUAL(psps -> log2_diff_max_min_luma_coding_block_size, 3);
  BOOST_CHECK_EQUAL(psps -> log2_min_transform_block_size_minus2, 0);
  BOOST_CHECK_EQUAL(psps -> log2_diff_max_min_transform_block_size, 3);
  BOOST_CHECK_EQUAL(psps -> max_transform_hierarchy_depth_inter, 2);
  BOOST_CHECK_EQUAL(psps -> max_transform_hierarchy_depth_intra, 2);

  BOOST_CHECK_EQUAL(psps -> scaling_list_enabled_flag, 0);
  BOOST_CHECK_EQUAL(psps -> amp_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> sample_adaptive_offset_enabled_flag, 0);
  BOOST_CHECK_EQUAL(psps -> pcm_enabled_flag, 0);
  BOOST_CHECK_EQUAL(psps -> num_short_term_ref_pic_sets, 5);
  BOOST_CHECK_EQUAL(psps -> short_term_ref_pic_set[0].num_negative_pics, 2);
  BOOST_CHECK_EQUAL(psps -> short_term_ref_pic_set[0].num_positive_pics, 0);

  BOOST_CHECK_EQUAL(psps -> long_term_ref_pics_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_mvp_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> strong_intra_smoothing_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sps_extension_flag, 0);

  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_info.m_position, 0x0000005B);
  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_pnalu -> getType(), NAL_PPS);

  std::shared_ptr<PPS> ppps = std::static_pointer_cast<PPS>(consumer.m_nalus[2].m_pnalu);
  BOOST_CHECK_EQUAL(ppps -> pps_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_seq_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> dependent_slice_segments_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> output_flag_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> num_extra_slice_header_bits, 0);
  BOOST_CHECK_EQUAL(ppps -> sign_data_hiding_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> cabac_init_present_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> num_ref_idx_l0_default_active_minus1, 1);
  BOOST_CHECK_EQUAL(ppps -> num_ref_idx_l1_default_active_minus1, 1);
  BOOST_CHECK_EQUAL(ppps -> init_qp_minus26, 0);
  BOOST_CHECK_EQUAL(ppps -> constrained_intra_pred_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> transform_skip_enabled_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> cu_qp_delta_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_cb_qp_offset, 5);
  BOOST_CHECK_EQUAL(ppps -> pps_cr_qp_offset, 5);
  BOOST_CHECK_EQUAL(ppps -> pps_slice_chroma_qp_offsets_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> weighted_pred_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> weighted_bipred_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> transquant_bypass_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> tiles_enabled_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> entropy_coding_sync_enabled_flag, 0);
  
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_info.m_position, 0x00000069);
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_pnalu -> getType(), NAL_IDR_W_RADL);
  std::shared_ptr<Slice> pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[3].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> no_output_of_prior_pics_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> dependent_slice_segment_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_reserved_undetermined_flag.size(), 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 2);

  
  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_info.m_position, 0x000002AE);
  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_pnalu -> getType(), NAL_TRAIL_R);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[4].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> dependent_slice_segment_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_reserved_undetermined_flag.size(), 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 1);
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_info.m_position, 0x000002C8);
  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_pnalu -> getType(), NAL_TSA_N);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[5].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> dependent_slice_segment_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_reserved_undetermined_flag.size(), 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 0);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 2);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 1);

  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_info.m_position, 0x000002E3);
  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_pnalu -> getType(), NAL_TSA_N);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[6].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> dependent_slice_segment_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_reserved_undetermined_flag.size(), 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 0);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 1);


  BOOST_CHECK_EQUAL(consumer.m_nalus[7].m_info.m_position, 0x000002FE);
  BOOST_CHECK_EQUAL(consumer.m_nalus[7].m_pnalu -> getType(), NAL_TSA_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[8].m_info.m_position, 0x00000319);
  BOOST_CHECK_EQUAL(consumer.m_nalus[8].m_pnalu -> getType(), NAL_TRAIL_R);
  BOOST_CHECK_EQUAL(consumer.m_nalus[9].m_info.m_position, 0x00000333);
  BOOST_CHECK_EQUAL(consumer.m_nalus[9].m_pnalu -> getType(), NAL_TSA_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[10].m_info.m_position, 0x0000034E);
  BOOST_CHECK_EQUAL(consumer.m_nalus[10].m_pnalu -> getType(), NAL_TSA_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[11].m_info.m_position, 0x00000369);
  BOOST_CHECK_EQUAL(consumer.m_nalus[11].m_pnalu -> getType(), NAL_TSA_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[12].m_info.m_position, 0x00000384);
  BOOST_CHECK_EQUAL(consumer.m_nalus[12].m_pnalu -> getType(), NAL_TRAIL_R);
  BOOST_CHECK_EQUAL(consumer.m_nalus[13].m_info.m_position, 0x0000039E);
  BOOST_CHECK_EQUAL(consumer.m_nalus[13].m_pnalu -> getType(), NAL_TSA_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[14].m_info.m_position, 0x000003B9);
  BOOST_CHECK_EQUAL(consumer.m_nalus[14].m_pnalu -> getType(), NAL_TSA_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[15].m_info.m_position, 0x000003D4);
  BOOST_CHECK_EQUAL(consumer.m_nalus[15].m_pnalu -> getType(), NAL_TSA_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[16].m_info.m_position, 0x000003EF);
  BOOST_CHECK_EQUAL(consumer.m_nalus[16].m_pnalu -> getType(), NAL_TRAIL_R);
  BOOST_CHECK_EQUAL(consumer.m_nalus[17].m_info.m_position, 0x00000409);
  BOOST_CHECK_EQUAL(consumer.m_nalus[17].m_pnalu -> getType(), NAL_TSA_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[18].m_info.m_position, 0x00000424);
  BOOST_CHECK_EQUAL(consumer.m_nalus[18].m_pnalu -> getType(), NAL_TSA_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[19].m_info.m_position, 0x0000043F);
  BOOST_CHECK_EQUAL(consumer.m_nalus[19].m_pnalu -> getType(), NAL_TSA_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[20].m_info.m_position, 0x0000045A);
  BOOST_CHECK_EQUAL(consumer.m_nalus[20].m_pnalu -> getType(), NAL_TRAIL_R);
  BOOST_CHECK_EQUAL(consumer.m_nalus[21].m_info.m_position, 0x00000474);
  BOOST_CHECK_EQUAL(consumer.m_nalus[21].m_pnalu -> getType(), NAL_TSA_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[22].m_info.m_position, 0x0000048F);
  BOOST_CHECK_EQUAL(consumer.m_nalus[22].m_pnalu -> getType(), NAL_TSA_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[23].m_info.m_position, 0x000004AA);
  BOOST_CHECK_EQUAL(consumer.m_nalus[23].m_pnalu -> getType(), NAL_TSA_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[24].m_info.m_position, 0x000004C5);
  BOOST_CHECK_EQUAL(consumer.m_nalus[24].m_pnalu -> getType(), NAL_TRAIL_R);
  BOOST_CHECK_EQUAL(consumer.m_nalus[25].m_info.m_position, 0x000004DF);
  BOOST_CHECK_EQUAL(consumer.m_nalus[25].m_pnalu -> getType(), NAL_TSA_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[26].m_info.m_position, 0x000004FA);
  BOOST_CHECK_EQUAL(consumer.m_nalus[26].m_pnalu -> getType(), NAL_TSA_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[27].m_info.m_position, 0x00000515);
  BOOST_CHECK_EQUAL(consumer.m_nalus[27].m_pnalu -> getType(), NAL_TSA_N);
  BOOST_CHECK_EQUAL(consumer.m_nalus[28].m_info.m_position, 0x00000530);
  BOOST_CHECK_EQUAL(consumer.m_nalus[28].m_pnalu -> getType(), NAL_TRAIL_R);
  BOOST_CHECK_EQUAL(consumer.m_nalus[29].m_info.m_position, 0x0000054A);
  BOOST_CHECK_EQUAL(consumer.m_nalus[29].m_pnalu -> getType(), NAL_TSA_N);

}



BOOST_AUTO_TEST_CASE(VIDEO_H265)
{
  Parser *pparser = Parser::create();
  
  Consumer consumer;
  
  pparser -> addConsumer(&consumer);
  
  std::ifstream in(getSourceDir() + "/samples/video-h265.hevc", std::ios::binary);
  
  in.seekg(0, std::ios::end);
  std::size_t size = in.tellg();
  in.seekg(0, std::ios::beg);
  
  char *pdata = new char[size];
  in.read(pdata, size);
  size = in.gcount();
  pparser -> process((const uint8_t *)pdata, size);
    
  pparser -> releaseConsumer(&consumer);
  Parser::release(pparser);
  

  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_info.m_position, 0x0);
  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_pnalu -> getType(), NAL_SEI_PREFIX);

  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_info.m_position, 0xA);
  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_pnalu -> getType(), NAL_VPS);

  std::shared_ptr<VPS> pvps = std::static_pointer_cast<VPS>(consumer.m_nalus[1].m_pnalu);
  
  BOOST_CHECK_EQUAL(pvps -> vps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_temporal_id_nesting_flag, 1);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_profile_space, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_tier_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_profile_idc, 1);

  uint8_t profile_compatibility[32] = { 0, 1, 1, 0};
  BOOST_CHECK_EQUAL_COLLECTIONS(profile_compatibility, profile_compatibility + 32, 
                              pvps -> profile_tier_level.general_profile_compatibility_flag, pvps -> profile_tier_level.general_profile_compatibility_flag+32);

  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_progressive_source_flag, 1);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_interlaced_source_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_non_packed_constraint_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_frame_only_constraint_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_level_idc, 120);
  
  BOOST_CHECK_EQUAL(pvps -> vps_sub_layer_ordering_info_present_flag, 1);

  uint8_t pic_buffering_minus1[1] = {4};
  uint8_t vps_max_num_reorder_pics[1] = {2};
  uint8_t latency_increase[1] = {0};

  BOOST_CHECK_EQUAL_COLLECTIONS(pic_buffering_minus1, pic_buffering_minus1 + 1,
		    pvps->vps_max_dec_pic_buffering_minus1.begin(), pvps->vps_max_dec_pic_buffering_minus1.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(vps_max_num_reorder_pics, vps_max_num_reorder_pics + 1,
		    pvps->vps_max_num_reorder_pics.begin(), pvps->vps_max_num_reorder_pics.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(latency_increase, latency_increase + 1,
		    pvps->vps_max_latency_increase_plus1.begin(), pvps->vps_max_latency_increase_plus1.end());
  
  BOOST_CHECK_EQUAL(pvps -> vps_max_layer_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_num_layer_sets_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_timing_info_present_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_extension_flag, 0);

  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_info.m_position, 0x26);
  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_pnalu -> getType(), NAL_SPS);

  std::shared_ptr<SPS> psps = std::static_pointer_cast<SPS>(consumer.m_nalus[2].m_pnalu);
  BOOST_CHECK_EQUAL(psps -> sps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(psps -> sps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_id_nesting_flag, 1);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_profile_space, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_tier_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_profile_idc, 1);
  BOOST_CHECK_EQUAL_COLLECTIONS(profile_compatibility, profile_compatibility + 32, 
                              psps -> profile_tier_level.general_profile_compatibility_flag, psps -> profile_tier_level.general_profile_compatibility_flag + 32);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_progressive_source_flag, 1);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_interlaced_source_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_non_packed_constraint_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_frame_only_constraint_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_level_idc, 120);
  
  BOOST_CHECK_EQUAL(psps -> sps_seq_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(psps -> chroma_format_idc, 1);

  BOOST_CHECK_EQUAL(psps -> pic_width_in_luma_samples, 1920);
  BOOST_CHECK_EQUAL(psps -> pic_height_in_luma_samples, 960);
  BOOST_CHECK_EQUAL(psps -> conformance_window_flag, 0);
  BOOST_CHECK_EQUAL(psps -> conf_win_left_offset, 0);
  BOOST_CHECK_EQUAL(psps -> conf_win_right_offset, 0);
  BOOST_CHECK_EQUAL(psps -> conf_win_top_offset, 0);
  BOOST_CHECK_EQUAL(psps -> conf_win_bottom_offset, 0);
  BOOST_CHECK_EQUAL(psps -> bit_depth_luma_minus8, 0);
  BOOST_CHECK_EQUAL(psps -> bit_depth_chroma_minus8, 0);
  BOOST_CHECK_EQUAL(psps -> log2_max_pic_order_cnt_lsb_minus4, 4);
  BOOST_CHECK_EQUAL(psps -> sps_sub_layer_ordering_info_present_flag, 1);

  uint8_t num_reorder_pics[1] = {2};
  BOOST_CHECK_EQUAL_COLLECTIONS(pic_buffering_minus1, pic_buffering_minus1 + 1,
		    psps->sps_max_dec_pic_buffering_minus1.begin(), psps->sps_max_dec_pic_buffering_minus1.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(num_reorder_pics, num_reorder_pics + 1,
		    psps->sps_max_num_reorder_pics.begin(), psps->sps_max_num_reorder_pics.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(latency_increase, latency_increase + 1,
		    psps->sps_max_latency_increase_plus1.begin(), psps->sps_max_latency_increase_plus1.end());

  BOOST_CHECK_EQUAL(psps -> log2_min_luma_coding_block_size_minus3, 0);
  BOOST_CHECK_EQUAL(psps -> log2_diff_max_min_luma_coding_block_size, 2);
  BOOST_CHECK_EQUAL(psps -> log2_min_transform_block_size_minus2, 0);
  BOOST_CHECK_EQUAL(psps -> log2_diff_max_min_transform_block_size, 3);
  BOOST_CHECK_EQUAL(psps -> max_transform_hierarchy_depth_inter, 0);
  BOOST_CHECK_EQUAL(psps -> max_transform_hierarchy_depth_intra, 0);

  BOOST_CHECK_EQUAL(psps -> scaling_list_enabled_flag, 0);
  BOOST_CHECK_EQUAL(psps -> amp_enabled_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sample_adaptive_offset_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> pcm_enabled_flag, 0);
  BOOST_CHECK_EQUAL(psps -> num_short_term_ref_pic_sets, 0);

  BOOST_CHECK_EQUAL(psps -> long_term_ref_pics_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_mvp_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> strong_intra_smoothing_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters_present_flag, 1);

  
  
  BOOST_CHECK_EQUAL(psps -> vui_parameters.aspect_ratio_info_present_flag, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.aspect_ratio_idc, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.overscan_info_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.video_signal_type_present_flag, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.video_format, 5);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.video_full_range_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.colour_description_present_flag, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.colour_primaries, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.transfer_characteristics, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.matrix_coeffs, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.chroma_loc_info_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.neutral_chroma_indication_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.field_seq_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.frame_field_info_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.default_display_window_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_timing_info_present_flag, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_num_units_in_tick, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_time_scale, 25);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_poc_proportional_to_timing_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.bitstream_restriction_flag, 0);
  
  BOOST_CHECK_EQUAL(psps -> sps_extension_flag, 0);

  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_info.m_position, 0x00000056);
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_pnalu -> getType(), NAL_PPS);

  std::shared_ptr<PPS> ppps = std::static_pointer_cast<PPS>(consumer.m_nalus[3].m_pnalu);
  BOOST_CHECK_EQUAL(ppps -> pps_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_seq_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> dependent_slice_segments_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> output_flag_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> num_extra_slice_header_bits, 0);
  BOOST_CHECK_EQUAL(ppps -> sign_data_hiding_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> cabac_init_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> num_ref_idx_l0_default_active_minus1, 0);
  BOOST_CHECK_EQUAL(ppps -> num_ref_idx_l1_default_active_minus1, 0);
  BOOST_CHECK_EQUAL(ppps -> init_qp_minus26, 0);
  BOOST_CHECK_EQUAL(ppps -> constrained_intra_pred_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> transform_skip_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> cu_qp_delta_enabled_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> pps_cb_qp_offset, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_cr_qp_offset, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_slice_chroma_qp_offsets_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> weighted_pred_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> weighted_bipred_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> transquant_bypass_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> tiles_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> entropy_coding_sync_enabled_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> loop_filter_across_tiles_enabled_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> deblocking_filter_control_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_scaling_list_data_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> lists_modification_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> log2_parallel_merge_level_minus2, 0);
  BOOST_CHECK_EQUAL(ppps -> slice_segment_header_extension_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_extension_flag, 0);
 
  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_info.m_position, 0x00000060);
  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_pnalu -> getType(), NAL_SEI_PREFIX);

  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_info.m_position, 0x00000381);
  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_pnalu -> getType(), NAL_IDR_W_RADL);

  std::shared_ptr<Slice> pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[5].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> no_output_of_prior_pics_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> dependent_slice_segment_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_reserved_undetermined_flag.size(), 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 2);

  BOOST_CHECK_EQUAL(pslice -> slice_sao_luma_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_sao_chroma_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, -9);
  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> num_entry_point_offsets, 29);
  BOOST_CHECK_EQUAL(pslice -> offset_len_minus1, 13);


  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_info.m_position, 0x000001C70D);
  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_pnalu -> getType(), NAL_TRAIL_R);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[6].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> dependent_slice_segment_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 1);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 5);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.num_negative_pics, 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.num_positive_pics, 0);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.delta_poc_s0_minus1[0], 4);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.used_by_curr_pic_s0_flag[0], 1);

  BOOST_CHECK_EQUAL(pslice -> slice_temporal_mvp_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_sao_luma_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_sao_chroma_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> num_ref_idx_active_override_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> pred_weight_table.luma_log2_weight_denom, 7);
  BOOST_CHECK_EQUAL(pslice -> pred_weight_table.delta_chroma_log2_weight_denom, -1);
  BOOST_CHECK_EQUAL(pslice -> pred_weight_table.luma_weight_l0_flag[0], 0);
  BOOST_CHECK_EQUAL(pslice -> pred_weight_table.chroma_weight_l0_flag[0], 0);
  BOOST_CHECK_EQUAL(pslice -> five_minus_max_num_merge_cand, 3);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, -5);
  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> num_entry_point_offsets, 29);
  BOOST_CHECK_EQUAL(pslice -> offset_len_minus1, 11);
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[7].m_info.m_position, 0x0000023E45);
  BOOST_CHECK_EQUAL(consumer.m_nalus[7].m_pnalu -> getType(), NAL_TRAIL_R);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[7].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> dependent_slice_segment_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 0);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 3);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.num_negative_pics, 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.num_positive_pics, 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.delta_poc_s0_minus1[0], 2);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.used_by_curr_pic_s0_flag[0], 1);

  BOOST_CHECK_EQUAL(pslice -> slice_temporal_mvp_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_sao_luma_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_sao_chroma_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> num_ref_idx_active_override_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> mvd_l1_zero_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> collocated_from_l0_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> five_minus_max_num_merge_cand, 3);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, -4);
  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> num_entry_point_offsets, 29);
  BOOST_CHECK_EQUAL(pslice -> offset_len_minus1, 10);  

  
  BOOST_CHECK_EQUAL(consumer.m_nalus[8].m_info.m_position, 0x0000027567);
  BOOST_CHECK_EQUAL(consumer.m_nalus[8].m_pnalu -> getType(), NAL_TRAIL_N);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[8].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> dependent_slice_segment_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 0);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.num_negative_pics, 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.num_positive_pics, 2);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.delta_poc_s0_minus1[0], 0);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.used_by_curr_pic_s0_flag[0], 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.delta_poc_s1_minus1[1], 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.used_by_curr_pic_s1_flag[1], 1);

  BOOST_CHECK_EQUAL(pslice -> slice_temporal_mvp_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_sao_luma_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_sao_chroma_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> num_ref_idx_active_override_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> num_ref_idx_l0_active_minus1, 0);
  BOOST_CHECK_EQUAL(pslice -> num_ref_idx_l1_active_minus1, 1);
  BOOST_CHECK_EQUAL(pslice -> mvd_l1_zero_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> collocated_from_l0_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> collocated_ref_idx, 0);
  BOOST_CHECK_EQUAL(pslice -> five_minus_max_num_merge_cand, 3);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, -2);
  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> num_entry_point_offsets, 29);
  BOOST_CHECK_EQUAL(pslice -> offset_len_minus1, 9);    
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[9].m_info.m_position, 0x0000029862);
  BOOST_CHECK_EQUAL(consumer.m_nalus[9].m_pnalu -> getType(), NAL_TRAIL_N);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[9].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> dependent_slice_segment_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 0);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 2);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.num_negative_pics, 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.num_positive_pics, 2);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.delta_poc_s0_minus1[0], 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.used_by_curr_pic_s0_flag[0], 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.delta_poc_s1_minus1[0], 0);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.used_by_curr_pic_s1_flag[1], 1);

  BOOST_CHECK_EQUAL(pslice -> slice_temporal_mvp_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_sao_luma_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_sao_chroma_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> num_ref_idx_active_override_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> num_ref_idx_l0_active_minus1, 0);
  BOOST_CHECK_EQUAL(pslice -> num_ref_idx_l1_active_minus1, 1);
  BOOST_CHECK_EQUAL(pslice -> mvd_l1_zero_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> collocated_from_l0_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> collocated_ref_idx, 0);
  BOOST_CHECK_EQUAL(pslice -> five_minus_max_num_merge_cand, 3);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, -2);
  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> num_entry_point_offsets, 29);
  BOOST_CHECK_EQUAL(pslice -> offset_len_minus1, 9);    

  BOOST_CHECK_EQUAL(consumer.m_nalus[10].m_info.m_position, 0x000002BE23);
  BOOST_CHECK_EQUAL(consumer.m_nalus[10].m_pnalu -> getType(), NAL_TRAIL_N);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[10].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> dependent_slice_segment_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 0);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 4);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.num_negative_pics, 2);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.num_positive_pics, 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.delta_poc_s0_minus1[0], 0);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.used_by_curr_pic_s0_flag[0], 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.delta_poc_s0_minus1[1], 2);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.used_by_curr_pic_s0_flag[1], 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.delta_poc_s1_minus1[0], 0);

  BOOST_CHECK_EQUAL(pslice -> slice_temporal_mvp_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_sao_luma_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_sao_chroma_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> num_ref_idx_active_override_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> mvd_l1_zero_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> collocated_from_l0_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> five_minus_max_num_merge_cand, 3);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, -2);
  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> num_entry_point_offsets, 29);
  BOOST_CHECK_EQUAL(pslice -> offset_len_minus1, 9);   
}


BOOST_AUTO_TEST_CASE(E_11345V_4T2_record_part)
{
  Parser *pparser = Parser::create();
  
  Consumer consumer;
  
  pparser -> addConsumer(&consumer);
  
  std::ifstream in(getSourceDir() + "/samples/10E_11345V_4T2_record_part.265", std::ios::binary);
  
  in.seekg(0, std::ios::end);
  std::size_t size = in.tellg();
  in.seekg(0, std::ios::beg);
  
  char *pdata = new char[size];
  in.read(pdata, size);
  size = in.gcount();
  pparser -> process((const uint8_t *)pdata, size);
    
  pparser -> releaseConsumer(&consumer);
  Parser::release(pparser);
  

  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_info.m_position, 0x0);
  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_pnalu -> getType(), NAL_AUD);

  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_info.m_position, 0x7);
  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_pnalu -> getType(), NAL_VPS);

  std::shared_ptr<VPS> pvps = std::static_pointer_cast<VPS>(consumer.m_nalus[1].m_pnalu);
  
  BOOST_CHECK_EQUAL(pvps -> vps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_temporal_id_nesting_flag, 1);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_profile_space, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_tier_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_profile_idc, 1);

  uint8_t profile_compatibility[32] = { 0, 1, 1, 0};
  BOOST_CHECK_EQUAL_COLLECTIONS(profile_compatibility, profile_compatibility + 32, 
                              pvps -> profile_tier_level.general_profile_compatibility_flag, pvps -> profile_tier_level.general_profile_compatibility_flag+32);

  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_progressive_source_flag, 1);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_interlaced_source_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_non_packed_constraint_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_frame_only_constraint_flag, 1);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_level_idc, 150);
  
  BOOST_CHECK_EQUAL(pvps -> vps_sub_layer_ordering_info_present_flag, 0);

  uint8_t pic_buffering_minus1[1] = {5};
  uint8_t vps_max_num_reorder_pics[1] = {3};
  uint8_t latency_increase[1] = {8};

  BOOST_CHECK_EQUAL_COLLECTIONS(pic_buffering_minus1, pic_buffering_minus1 + 1,
		    pvps->vps_max_dec_pic_buffering_minus1.begin(), pvps->vps_max_dec_pic_buffering_minus1.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(vps_max_num_reorder_pics, vps_max_num_reorder_pics + 1,
		    pvps->vps_max_num_reorder_pics.begin(), pvps->vps_max_num_reorder_pics.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(latency_increase, latency_increase + 1,
		    pvps->vps_max_latency_increase_plus1.begin(), pvps->vps_max_latency_increase_plus1.end());
  
  BOOST_CHECK_EQUAL(pvps -> vps_max_layer_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_num_layer_sets_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_timing_info_present_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_extension_flag, 0);

  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_info.m_position, 0x24);
  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_pnalu -> getType(), NAL_SPS);

  std::shared_ptr<SPS> psps = std::static_pointer_cast<SPS>(consumer.m_nalus[2].m_pnalu);
  BOOST_CHECK_EQUAL(psps -> sps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(psps -> sps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_id_nesting_flag, 1);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_profile_space, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_tier_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_profile_idc, 1);
  BOOST_CHECK_EQUAL_COLLECTIONS(profile_compatibility, profile_compatibility + 32, 
                              psps -> profile_tier_level.general_profile_compatibility_flag, psps -> profile_tier_level.general_profile_compatibility_flag + 32);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_progressive_source_flag, 1);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_interlaced_source_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_non_packed_constraint_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_frame_only_constraint_flag, 1);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_level_idc, 150);
  
  BOOST_CHECK_EQUAL(psps -> sps_seq_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(psps -> chroma_format_idc, 1);

  BOOST_CHECK_EQUAL(psps -> pic_width_in_luma_samples, 3840);
  BOOST_CHECK_EQUAL(psps -> pic_height_in_luma_samples, 2160);
  BOOST_CHECK_EQUAL(psps -> conformance_window_flag, 0);
  BOOST_CHECK_EQUAL(psps -> bit_depth_luma_minus8, 0);
  BOOST_CHECK_EQUAL(psps -> bit_depth_chroma_minus8, 0);
  BOOST_CHECK_EQUAL(psps -> log2_max_pic_order_cnt_lsb_minus4, 2);
  BOOST_CHECK_EQUAL(psps -> sps_sub_layer_ordering_info_present_flag, 0);

  uint8_t num_reorder_pics[1] = {3};
  BOOST_CHECK_EQUAL_COLLECTIONS(pic_buffering_minus1, pic_buffering_minus1 + 1,
		    psps->sps_max_dec_pic_buffering_minus1.begin(), psps->sps_max_dec_pic_buffering_minus1.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(num_reorder_pics, num_reorder_pics + 1,
		    psps->sps_max_num_reorder_pics.begin(), psps->sps_max_num_reorder_pics.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(latency_increase, latency_increase + 1,
		    psps->sps_max_latency_increase_plus1.begin(), psps->sps_max_latency_increase_plus1.end());

  BOOST_CHECK_EQUAL(psps -> log2_min_luma_coding_block_size_minus3, 0);
  BOOST_CHECK_EQUAL(psps -> log2_diff_max_min_luma_coding_block_size, 3);
  BOOST_CHECK_EQUAL(psps -> log2_min_transform_block_size_minus2, 0);
  BOOST_CHECK_EQUAL(psps -> log2_diff_max_min_transform_block_size, 3);
  BOOST_CHECK_EQUAL(psps -> max_transform_hierarchy_depth_inter, 4);
  BOOST_CHECK_EQUAL(psps -> max_transform_hierarchy_depth_intra, 4);

  BOOST_CHECK_EQUAL(psps -> scaling_list_enabled_flag, 0);
  BOOST_CHECK_EQUAL(psps -> amp_enabled_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sample_adaptive_offset_enabled_flag, 0);
  BOOST_CHECK_EQUAL(psps -> pcm_enabled_flag, 0);
  BOOST_CHECK_EQUAL(psps -> num_short_term_ref_pic_sets, 0);

  BOOST_CHECK_EQUAL(psps -> long_term_ref_pics_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_mvp_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> strong_intra_smoothing_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters_present_flag, 1);

  
  
  BOOST_CHECK_EQUAL(psps -> vui_parameters.aspect_ratio_info_present_flag, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.aspect_ratio_idc, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.overscan_info_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.video_signal_type_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.chroma_loc_info_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.neutral_chroma_indication_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.field_seq_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.frame_field_info_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.default_display_window_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_timing_info_present_flag, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_num_units_in_tick, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_time_scale, 50);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_poc_proportional_to_timing_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.bitstream_restriction_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sps_extension_flag, 0);

  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_info.m_position, 0x00000053);
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_pnalu -> getType(), NAL_PPS);

  std::shared_ptr<PPS> ppps = std::static_pointer_cast<PPS>(consumer.m_nalus[3].m_pnalu);
  BOOST_CHECK_EQUAL(ppps -> pps_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_seq_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> dependent_slice_segments_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> output_flag_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> num_extra_slice_header_bits, 0);
  BOOST_CHECK_EQUAL(ppps -> sign_data_hiding_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> cabac_init_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> num_ref_idx_l0_default_active_minus1, 0);
  BOOST_CHECK_EQUAL(ppps -> num_ref_idx_l1_default_active_minus1, 0);
  BOOST_CHECK_EQUAL(ppps -> init_qp_minus26, 0);
  BOOST_CHECK_EQUAL(ppps -> constrained_intra_pred_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> transform_skip_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> cu_qp_delta_enabled_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> diff_cu_qp_delta_depth, 2);
  BOOST_CHECK_EQUAL(ppps -> pps_cb_qp_offset, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_cr_qp_offset, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_slice_chroma_qp_offsets_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> weighted_pred_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> weighted_bipred_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> transquant_bypass_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> tiles_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> entropy_coding_sync_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_loop_filter_across_slices_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> deblocking_filter_control_present_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> deblocking_filter_override_enabled_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> pps_deblocking_filter_disabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_beta_offset_div2, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_tc_offset_div2, 0);
  
  BOOST_CHECK_EQUAL(ppps -> pps_scaling_list_data_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> lists_modification_present_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> log2_parallel_merge_level_minus2, 0);
  BOOST_CHECK_EQUAL(ppps -> slice_segment_header_extension_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_extension_flag, 0);
 
  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_info.m_position, 0x0000005E);
  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_pnalu -> getType(), NAL_CRA_NUT);

  std::shared_ptr<Slice> pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[4].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> no_output_of_prior_pics_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> dependent_slice_segment_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 2);

  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 20);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 0);

  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.num_negative_pics, 4);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.num_positive_pics, 0);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.delta_poc_s0_minus1[0], 5);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.used_by_curr_pic_s0_flag[0], 0);

  BOOST_CHECK_EQUAL(pslice -> slice_temporal_mvp_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, -10);
  BOOST_CHECK_EQUAL(pslice -> deblocking_filter_override_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_deblocking_filter_disabled_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_beta_offset_div2, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_tc_offset_div2, 0);

  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_info.m_position, 0x000003C193);
  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_pnalu -> getType(), NAL_AUD);

  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_info.m_position, 0x000003C19A);
  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_pnalu -> getType(), NAL_RASL_R);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[6].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 0);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 17);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.num_negative_pics, 3);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.num_positive_pics, 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.delta_poc_s0_minus1[0], 2);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.used_by_curr_pic_s0_flag[0], 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.delta_poc_s0_minus1[1], 7);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.used_by_curr_pic_s0_flag[1], 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.delta_poc_s0_minus1[2], 7);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set.used_by_curr_pic_s0_flag[2], 0);

  BOOST_CHECK_EQUAL(pslice -> slice_temporal_mvp_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> num_ref_idx_active_override_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> num_ref_idx_l0_active_minus1, 1);
  BOOST_CHECK_EQUAL(pslice -> num_ref_idx_l1_active_minus1, 0);
  BOOST_CHECK_EQUAL(pslice -> mvd_l1_zero_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> collocated_from_l0_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> collocated_ref_idx, 0);
  BOOST_CHECK_EQUAL(pslice -> pred_weight_table.luma_log2_weight_denom, 6);
  BOOST_CHECK_EQUAL(pslice -> pred_weight_table.delta_chroma_log2_weight_denom, 0);
  BOOST_CHECK_EQUAL(pslice -> pred_weight_table.luma_weight_l0_flag[0], 0);
  BOOST_CHECK_EQUAL(pslice -> pred_weight_table.luma_weight_l0_flag[1], 0);
  BOOST_CHECK_EQUAL(pslice -> pred_weight_table.chroma_weight_l0_flag[0], 0);
  BOOST_CHECK_EQUAL(pslice -> pred_weight_table.chroma_weight_l0_flag[1], 0);
  BOOST_CHECK_EQUAL(pslice -> pred_weight_table.luma_weight_l1_flag[0], 0);
  BOOST_CHECK_EQUAL(pslice -> pred_weight_table.chroma_weight_l1_flag[0], 0);

  BOOST_CHECK_EQUAL(pslice -> five_minus_max_num_merge_cand, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, -2);
  
  BOOST_CHECK_EQUAL(pslice -> deblocking_filter_override_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_deblocking_filter_disabled_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_beta_offset_div2, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_tc_offset_div2, 0);
}


BOOST_AUTO_TEST_CASE(TearsOfSteel_720p_h265_part)
{
  Parser *pparser = Parser::create();
  
  Consumer consumer;
  
  pparser -> addConsumer(&consumer);
  
  std::ifstream in(getSourceDir() + "/samples/TearsOfSteel_720p_h265_part.hevc", std::ios::binary);
  
  in.seekg(0, std::ios::end);
  std::size_t size = in.tellg();
  in.seekg(0, std::ios::beg);
  
  char *pdata = new char[size];
  in.read(pdata, size);
  size = in.gcount();
  pparser -> process((const uint8_t *)pdata, size);
    
  pparser -> releaseConsumer(&consumer);
  Parser::release(pparser);
  

  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_info.m_position, 0x0);
  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_pnalu -> getType(), NAL_VPS);

  std::shared_ptr<VPS> pvps = std::static_pointer_cast<VPS>(consumer.m_nalus[0].m_pnalu);
  
  BOOST_CHECK_EQUAL(pvps -> vps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_temporal_id_nesting_flag, 1);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_profile_space, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_tier_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_profile_idc, 1);

  uint8_t profile_compatibility[32] = { 0, 1, 1, 0};
  BOOST_CHECK_EQUAL_COLLECTIONS(profile_compatibility, profile_compatibility + 32, 
                              pvps -> profile_tier_level.general_profile_compatibility_flag, pvps -> profile_tier_level.general_profile_compatibility_flag+32);

  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_progressive_source_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_interlaced_source_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_non_packed_constraint_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_frame_only_constraint_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_level_idc, 186);
  
  BOOST_CHECK_EQUAL(pvps -> vps_sub_layer_ordering_info_present_flag, 1);

  uint8_t pic_buffering_minus1[1] = {4};
  uint8_t vps_max_num_reorder_pics[1] = {3};
  uint8_t latency_increase[1] = {0};

  BOOST_CHECK_EQUAL_COLLECTIONS(pic_buffering_minus1, pic_buffering_minus1 + 1,
		    pvps->vps_max_dec_pic_buffering_minus1.begin(), pvps->vps_max_dec_pic_buffering_minus1.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(vps_max_num_reorder_pics, vps_max_num_reorder_pics + 1,
		    pvps->vps_max_num_reorder_pics.begin(), pvps->vps_max_num_reorder_pics.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(latency_increase, latency_increase + 1,
		    pvps->vps_max_latency_increase_plus1.begin(), pvps->vps_max_latency_increase_plus1.end());
  
  BOOST_CHECK_EQUAL(pvps -> vps_max_layer_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_num_layer_sets_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_timing_info_present_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_extension_flag, 0);

  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_info.m_position, 0x1d);
  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_pnalu -> getType(), NAL_SPS);

  std::shared_ptr<SPS> psps = std::static_pointer_cast<SPS>(consumer.m_nalus[1].m_pnalu);
  BOOST_CHECK_EQUAL(psps -> sps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(psps -> sps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_id_nesting_flag, 1);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_profile_space, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_tier_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_profile_idc, 1);
  BOOST_CHECK_EQUAL_COLLECTIONS(profile_compatibility, profile_compatibility + 32, 
                              psps -> profile_tier_level.general_profile_compatibility_flag, psps -> profile_tier_level.general_profile_compatibility_flag + 32);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_progressive_source_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_interlaced_source_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_non_packed_constraint_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_frame_only_constraint_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_level_idc, 186);
  
  BOOST_CHECK_EQUAL(psps -> sps_seq_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(psps -> chroma_format_idc, 1);

  BOOST_CHECK_EQUAL(psps -> pic_width_in_luma_samples, 1280);
  BOOST_CHECK_EQUAL(psps -> pic_height_in_luma_samples, 720);
  BOOST_CHECK_EQUAL(psps -> long_term_ref_pics_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_mvp_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> strong_intra_smoothing_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sps_extension_flag, 0);

  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_info.m_position, 0x00000052);
  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_pnalu -> getType(), NAL_PPS);

  std::shared_ptr<PPS> ppps = std::static_pointer_cast<PPS>(consumer.m_nalus[2].m_pnalu);
  BOOST_CHECK_EQUAL(ppps -> pps_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_seq_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> dependent_slice_segments_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> output_flag_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> num_extra_slice_header_bits, 0);
  BOOST_CHECK_EQUAL(ppps -> sign_data_hiding_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> cabac_init_present_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> num_ref_idx_l0_default_active_minus1, 1);
  BOOST_CHECK_EQUAL(ppps -> num_ref_idx_l1_default_active_minus1, 1);
  BOOST_CHECK_EQUAL(ppps -> init_qp_minus26, 0);
  BOOST_CHECK_EQUAL(ppps -> constrained_intra_pred_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> transform_skip_enabled_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> cu_qp_delta_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_cb_qp_offset, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_cr_qp_offset, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_slice_chroma_qp_offsets_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> weighted_pred_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> weighted_bipred_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> transquant_bypass_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> tiles_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> entropy_coding_sync_enabled_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> pps_loop_filter_across_slices_enabled_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> deblocking_filter_control_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_scaling_list_data_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> lists_modification_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> log2_parallel_merge_level_minus2, 0);
  BOOST_CHECK_EQUAL(ppps -> slice_segment_header_extension_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_extension_flag, 0);
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_info.m_position, 0x5d);
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_pnalu -> getType(), NAL_VPS);

  pvps = std::static_pointer_cast<VPS>(consumer.m_nalus[3].m_pnalu);
  
  BOOST_CHECK_EQUAL(pvps -> vps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_temporal_id_nesting_flag, 1);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_profile_space, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_tier_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_profile_idc, 1);

  BOOST_CHECK_EQUAL_COLLECTIONS(profile_compatibility, profile_compatibility + 32, 
                              pvps -> profile_tier_level.general_profile_compatibility_flag, pvps -> profile_tier_level.general_profile_compatibility_flag+32);

  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_progressive_source_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_interlaced_source_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_non_packed_constraint_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_frame_only_constraint_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_level_idc, 186);
  
  BOOST_CHECK_EQUAL(pvps -> vps_sub_layer_ordering_info_present_flag, 1);

  BOOST_CHECK_EQUAL_COLLECTIONS(pic_buffering_minus1, pic_buffering_minus1 + 1,
		    pvps->vps_max_dec_pic_buffering_minus1.begin(), pvps->vps_max_dec_pic_buffering_minus1.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(vps_max_num_reorder_pics, vps_max_num_reorder_pics + 1,
		    pvps->vps_max_num_reorder_pics.begin(), pvps->vps_max_num_reorder_pics.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(latency_increase, latency_increase + 1,
		    pvps->vps_max_latency_increase_plus1.begin(), pvps->vps_max_latency_increase_plus1.end());
  
  BOOST_CHECK_EQUAL(pvps -> vps_max_layer_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_num_layer_sets_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_timing_info_present_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_extension_flag, 0);

  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_info.m_position, 0x7a);
  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_pnalu -> getType(), NAL_SPS);

  psps = std::static_pointer_cast<SPS>(consumer.m_nalus[4].m_pnalu);
  BOOST_CHECK_EQUAL(psps -> sps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(psps -> sps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_id_nesting_flag, 1);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_profile_space, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_tier_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_profile_idc, 1);
  BOOST_CHECK_EQUAL_COLLECTIONS(profile_compatibility, profile_compatibility + 32, 
                              psps -> profile_tier_level.general_profile_compatibility_flag, psps -> profile_tier_level.general_profile_compatibility_flag + 32);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_progressive_source_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_interlaced_source_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_non_packed_constraint_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_frame_only_constraint_flag, 0);
  BOOST_CHECK_EQUAL(psps -> profile_tier_level.general_level_idc, 186);
  
  BOOST_CHECK_EQUAL(psps -> sps_seq_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(psps -> chroma_format_idc, 1);

  BOOST_CHECK_EQUAL(psps -> pic_width_in_luma_samples, 1280);
  BOOST_CHECK_EQUAL(psps -> pic_height_in_luma_samples, 720);
  BOOST_CHECK_EQUAL(psps -> long_term_ref_pics_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_mvp_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> strong_intra_smoothing_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sps_extension_flag, 0);

  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_info.m_position, 0x000000af);
  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_pnalu -> getType(), NAL_PPS);

  ppps = std::static_pointer_cast<PPS>(consumer.m_nalus[5].m_pnalu);
  BOOST_CHECK_EQUAL(ppps -> pps_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_seq_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> dependent_slice_segments_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> output_flag_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> num_extra_slice_header_bits, 0);
  BOOST_CHECK_EQUAL(ppps -> sign_data_hiding_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> cabac_init_present_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> num_ref_idx_l0_default_active_minus1, 1);
  BOOST_CHECK_EQUAL(ppps -> num_ref_idx_l1_default_active_minus1, 1);
  BOOST_CHECK_EQUAL(ppps -> init_qp_minus26, 0);
  BOOST_CHECK_EQUAL(ppps -> constrained_intra_pred_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> transform_skip_enabled_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> cu_qp_delta_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_cb_qp_offset, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_cr_qp_offset, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_slice_chroma_qp_offsets_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> weighted_pred_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> weighted_bipred_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> transquant_bypass_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> tiles_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> entropy_coding_sync_enabled_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> pps_loop_filter_across_slices_enabled_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> deblocking_filter_control_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_scaling_list_data_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> lists_modification_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> log2_parallel_merge_level_minus2, 0);
  BOOST_CHECK_EQUAL(ppps -> slice_segment_header_extension_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_extension_flag, 0);
  
  
  
 
  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_info.m_position, 0x000000ba);
  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_pnalu -> getType(), NAL_IDR_W_RADL);

  std::shared_ptr<Slice> pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[6].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> no_output_of_prior_pics_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 2);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, 1);

  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> num_entry_point_offsets, 11);
  BOOST_CHECK_EQUAL(pslice -> offset_len_minus1, 4);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[0], 20);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[1], 14);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[2], 12);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[3], 11);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[4], 9);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[5], 8);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[6], 8);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[7], 8);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[8], 7);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[9], 6);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[10], 6);


  
  BOOST_CHECK_EQUAL(consumer.m_nalus[7].m_info.m_position, 0x000015a);
  BOOST_CHECK_EQUAL(consumer.m_nalus[7].m_pnalu -> getType(), NAL_TRAIL_R);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[7].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 0);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 8);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 1);

  BOOST_CHECK_EQUAL(pslice -> num_entry_point_offsets, 11);
  BOOST_CHECK_EQUAL(pslice -> offset_len_minus1, 2);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[0], 3);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[1], 2);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[2], 2);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[3], 2);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[4], 2);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[5], 2);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[6], 1);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[7], 1);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[8], 1);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[9], 1);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[10], 1);



  BOOST_CHECK_EQUAL(consumer.m_nalus[8].m_info.m_position, 0x000018a);
  BOOST_CHECK_EQUAL(consumer.m_nalus[8].m_pnalu -> getType(), NAL_TRAIL_R);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[8].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 0);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 4);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 1);

  BOOST_CHECK_EQUAL(pslice -> num_entry_point_offsets, 11);
  BOOST_CHECK_EQUAL(pslice -> offset_len_minus1, 1);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[0], 2);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[1], 2);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[2], 2);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[3], 2);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[4], 2);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[5], 1);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[6], 1);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[7], 1);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[8], 1);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[9], 1);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[10], 1);
}


BOOST_AUTO_TEST_CASE(Jellyfish_3_Mbps_1080p_hevc_part)
{
  Parser *pparser = Parser::create();
  
  Consumer consumer;
  
  pparser -> addConsumer(&consumer);
  
  std::ifstream in(getSourceDir() + "/samples/Jellyfish-3-Mbps-1080p-hevc_part.hevc", std::ios::binary);
  
  in.seekg(0, std::ios::end);
  std::size_t size = in.tellg();
  in.seekg(0, std::ios::beg);
  
  char *pdata = new char[size];
  in.read(pdata, size);
  size = in.gcount();
  pparser -> process((const uint8_t *)pdata, size);
    
  pparser -> releaseConsumer(&consumer);
  Parser::release(pparser);
  

  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_info.m_position, 0x0);
  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_pnalu -> getType(), NAL_SEI_PREFIX);

  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_info.m_position, 0x3b2);
  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_pnalu -> getType(), NAL_VPS);
  
  std::shared_ptr<VPS> pvps = std::static_pointer_cast<VPS>(consumer.m_nalus[1].m_pnalu);
  
  BOOST_CHECK_EQUAL(pvps -> vps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_temporal_id_nesting_flag, 1);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_level_idc, 120);
  BOOST_CHECK_EQUAL(pvps -> vps_max_layer_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_num_layer_sets_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_timing_info_present_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_extension_flag, 0);
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_info.m_position, 0x3cf);
  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_pnalu -> getType(), NAL_SPS);

  std::shared_ptr<SPS> psps = std::static_pointer_cast<SPS>(consumer.m_nalus[2].m_pnalu);
  BOOST_CHECK_EQUAL(psps -> sps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(psps -> sps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_id_nesting_flag, 1);

  BOOST_CHECK_EQUAL(psps -> pic_width_in_luma_samples, 1920);
  BOOST_CHECK_EQUAL(psps -> pic_height_in_luma_samples, 1080);

  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_num_units_in_tick, 125);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_time_scale, 2997);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_poc_proportional_to_timing_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_hrd_parameters_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.bitstream_restriction_flag, 0);

  BOOST_CHECK_EQUAL(psps -> sps_extension_flag, 0);

  
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_info.m_position, 0x000003fd);
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_pnalu -> getType(), NAL_PPS);

  std::shared_ptr<PPS> ppps = std::static_pointer_cast<PPS>(consumer.m_nalus[3].m_pnalu);
  BOOST_CHECK_EQUAL(ppps -> pps_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_seq_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> dependent_slice_segments_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> output_flag_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> num_extra_slice_header_bits, 0);
  BOOST_CHECK_EQUAL(ppps -> sign_data_hiding_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> deblocking_filter_control_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_scaling_list_data_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> lists_modification_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> log2_parallel_merge_level_minus2, 0);
  BOOST_CHECK_EQUAL(ppps -> slice_segment_header_extension_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_extension_flag, 0);
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_info.m_position, 0x408);
  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_pnalu -> getType(), NAL_IDR_W_RADL);

  std::shared_ptr<Slice> pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[4].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> no_output_of_prior_pics_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 2);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, 9);

  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> num_entry_point_offsets, 16);
  BOOST_CHECK_EQUAL(pslice -> offset_len_minus1, 11);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[0], 45);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[1], 34);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[15], 57	);  


  
  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_info.m_position, 0x0004d9b);
  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_pnalu -> getType(), NAL_TRAIL_R);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[5].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 1);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 4);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 0);

  BOOST_CHECK_EQUAL(pslice -> num_entry_point_offsets, 16);
  BOOST_CHECK_EQUAL(pslice -> offset_len_minus1, 10);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[0], 5);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[1], 7);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[12], 145);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[13], 67);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[14], 72);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[15], 5);  



  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_info.m_position, 0x0007c3e);
  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_pnalu -> getType(), NAL_TRAIL_R);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[6].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 0);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 2);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 0);

  BOOST_CHECK_EQUAL(pslice -> num_entry_point_offsets, 16);
  BOOST_CHECK_EQUAL(pslice -> offset_len_minus1, 9);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[0], 4);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[1], 6);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[2], 110);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[12], 48);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[13], 9);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[14], 10);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[15], 6); 
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[7].m_info.m_position, 0x0008e5d);
  BOOST_CHECK_EQUAL(consumer.m_nalus[7].m_pnalu -> getType(), NAL_TRAIL_N);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[7].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 0);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 0);

  BOOST_CHECK_EQUAL(pslice -> num_entry_point_offsets, 16);
  BOOST_CHECK_EQUAL(pslice -> offset_len_minus1, 7);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[0], 4);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[1], 6);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[2], 23);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[3], 64);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[4], 47);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[5], 190);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[6], 147);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[7], 179);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[8], 92);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[9], 127);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[10], 74);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[11], 68);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[12], 7);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[13], 9);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[14], 3);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[15], 2);
}


BOOST_AUTO_TEST_CASE(ffmpeg_default)
{
  Parser *pparser = Parser::create();
  
  Consumer consumer;
  
  pparser -> addConsumer(&consumer);
  
  std::ifstream in(getSourceDir() + "/samples/ffmpeg_default.hevc", std::ios::binary);
  
  in.seekg(0, std::ios::end);
  std::size_t size = in.tellg();
  in.seekg(0, std::ios::beg);
  
  char *pdata = new char[size];
  in.read(pdata, size);
  size = in.gcount();
  pparser -> process((const uint8_t *)pdata, size);
    
  pparser -> releaseConsumer(&consumer);
  Parser::release(pparser);
  

  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_info.m_position, 0x0);
  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_pnalu -> getType(), NAL_VPS);
  
  std::shared_ptr<VPS> pvps = std::static_pointer_cast<VPS>(consumer.m_nalus[0].m_pnalu);
  
  BOOST_CHECK_EQUAL(pvps -> vps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_temporal_id_nesting_flag, 1);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_level_idc, 120);
  BOOST_CHECK_EQUAL(pvps -> vps_max_layer_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_num_layer_sets_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_timing_info_present_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_extension_flag, 0);
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_info.m_position, 0x1c);
  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_pnalu -> getType(), NAL_SPS);

  std::shared_ptr<SPS> psps = std::static_pointer_cast<SPS>(consumer.m_nalus[1].m_pnalu);
  BOOST_CHECK_EQUAL(psps -> sps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(psps -> sps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_id_nesting_flag, 1);

  BOOST_CHECK_EQUAL(psps -> pic_width_in_luma_samples, 1920);
  BOOST_CHECK_EQUAL(psps -> pic_height_in_luma_samples, 1080);

  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_num_units_in_tick, 125);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_time_scale, 2997);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_poc_proportional_to_timing_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_hrd_parameters_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.bitstream_restriction_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sps_extension_flag, 0);

  
  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_info.m_position, 0x0000048);
  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_pnalu -> getType(), NAL_PPS);

  std::shared_ptr<PPS> ppps = std::static_pointer_cast<PPS>(consumer.m_nalus[2].m_pnalu);
  BOOST_CHECK_EQUAL(ppps -> pps_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_seq_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> dependent_slice_segments_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> output_flag_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> num_extra_slice_header_bits, 0);
  BOOST_CHECK_EQUAL(ppps -> sign_data_hiding_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> deblocking_filter_control_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_scaling_list_data_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> lists_modification_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> log2_parallel_merge_level_minus2, 0);
  BOOST_CHECK_EQUAL(ppps -> slice_segment_header_extension_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_extension_flag, 0);
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_info.m_position, 0x52);
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_pnalu -> getType(), NAL_SEI_PREFIX);

  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_info.m_position, 0x3c0);
  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_pnalu -> getType(), NAL_SEI_PREFIX);

  std::shared_ptr<SEI> psei = std::static_pointer_cast<SEI>(consumer.m_nalus[4].m_pnalu);
  
  BOOST_CHECK_EQUAL(psei -> sei_message.size(), 1);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_type_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_type_byte, 6);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_size_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_size_byte, 1);

  std::shared_ptr<RecoveryPoint> precPoint = std::static_pointer_cast<RecoveryPoint>(psei -> sei_message[0].sei_payload);
  BOOST_CHECK_EQUAL(precPoint -> recovery_poc_cnt, 0);
  BOOST_CHECK_EQUAL(precPoint -> exact_match_flag, 1);
  BOOST_CHECK_EQUAL(precPoint -> broken_link_flag, 0);

  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_info.m_position, 0x3c9);
  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_pnalu -> getType(), NAL_IDR_W_RADL);
  std::shared_ptr<Slice> pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[5].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> no_output_of_prior_pics_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 2);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, 7);

  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> num_entry_point_offsets, 16);
  BOOST_CHECK_EQUAL(pslice -> offset_len_minus1, 11);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[0], 24);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[1], 154);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[15], 20);  


  
  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_info.m_position, 0x579d);
  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_pnalu -> getType(), NAL_TRAIL_R);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[6].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 1);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 3);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 0);

  BOOST_CHECK_EQUAL(pslice -> num_entry_point_offsets, 16);
  BOOST_CHECK_EQUAL(pslice -> offset_len_minus1, 10);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[0], 6);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[1], 22);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[12], 184);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[13], 117);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[14], 88);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[15], 8);  



  BOOST_CHECK_EQUAL(consumer.m_nalus[7].m_info.m_position, 0x8d5d);
  BOOST_CHECK_EQUAL(consumer.m_nalus[7].m_pnalu -> getType(), NAL_TRAIL_R);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[7].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 0);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 2);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 0);

  BOOST_CHECK_EQUAL(pslice -> num_entry_point_offsets, 16);
  BOOST_CHECK_EQUAL(pslice -> offset_len_minus1, 9);
  BOOST_CHECK_EQUAL(pslice -> num_entry_point_offsets, 16);
  BOOST_CHECK_EQUAL(pslice -> offset_len_minus1, 9);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[0], 4);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[1], 8);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[13], 18);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[14], 16);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[15], 7);
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[8].m_info.m_position, 0xa56c);
  BOOST_CHECK_EQUAL(consumer.m_nalus[8].m_pnalu -> getType(), NAL_TRAIL_N);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[8].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 0);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 0);

  BOOST_CHECK_EQUAL(pslice -> num_entry_point_offsets, 16);
  BOOST_CHECK_EQUAL(pslice -> offset_len_minus1, 8);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[0], 4);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[1], 10);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[13], 16);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[14], 6);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[15], 6);
}




BOOST_AUTO_TEST_CASE(ffmpeg_cp_tr_cm_2020)
{
  Parser *pparser = Parser::create();
  
  Consumer consumer;
  
  pparser -> addConsumer(&consumer);
  
  std::ifstream in(getSourceDir() + "/samples/ffmpeg_cp_tr_cm_2020.hevc", std::ios::binary);
  
  in.seekg(0, std::ios::end);
  std::size_t size = in.tellg();
  in.seekg(0, std::ios::beg);
  
  char *pdata = new char[size];
  in.read(pdata, size);
  size = in.gcount();
  pparser -> process((const uint8_t *)pdata, size);
    
  pparser -> releaseConsumer(&consumer);
  Parser::release(pparser);
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_info.m_position, 0x1c);
  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_pnalu -> getType(), NAL_SPS);

  std::shared_ptr<SPS> psps = std::static_pointer_cast<SPS>(consumer.m_nalus[1].m_pnalu);
  BOOST_CHECK_EQUAL(psps -> sps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(psps -> sps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_id_nesting_flag, 1);

  BOOST_CHECK_EQUAL(psps -> pic_width_in_luma_samples, 1920);
  BOOST_CHECK_EQUAL(psps -> pic_height_in_luma_samples, 1080);

  BOOST_CHECK_EQUAL(psps -> vui_parameters.video_signal_type_present_flag, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.video_format, 5);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.video_full_range_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.colour_description_present_flag, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.colour_primaries, 2);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.transfer_characteristics, 2);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.matrix_coeffs, 10);

  
  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_num_units_in_tick, 125);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_time_scale, 2997);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_poc_proportional_to_timing_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.vui_hrd_parameters_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> vui_parameters.bitstream_restriction_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sps_extension_flag, 0);
}


BOOST_AUTO_TEST_CASE(f265_default)
{
  Parser *pparser = Parser::create();
  
  Consumer consumer;
  
  pparser -> addConsumer(&consumer);
  
  std::ifstream in(getSourceDir() + "/samples/f265_default.hevc", std::ios::binary);
  
  in.seekg(0, std::ios::end);
  std::size_t size = in.tellg();
  in.seekg(0, std::ios::beg);
  
  char *pdata = new char[size];
  in.read(pdata, size);
  size = in.gcount();
  pparser -> process((const uint8_t *)pdata, size);
    
  pparser -> releaseConsumer(&consumer);
  Parser::release(pparser);
  

  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_info.m_position, 0x0);
  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_pnalu -> getType(), NAL_VPS);
  
  std::shared_ptr<VPS> pvps = std::static_pointer_cast<VPS>(consumer.m_nalus[0].m_pnalu);
  
  BOOST_CHECK_EQUAL(pvps -> vps_video_parameter_set_id, 11);
  BOOST_CHECK_EQUAL(pvps -> vps_max_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_temporal_id_nesting_flag, 1);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_level_idc, 186);
  BOOST_CHECK_EQUAL(pvps -> vps_max_layer_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_num_layer_sets_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_timing_info_present_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_extension_flag, 0);
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_info.m_position, 0x1d);
  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_pnalu -> getType(), NAL_SPS);

  std::shared_ptr<SPS> psps = std::static_pointer_cast<SPS>(consumer.m_nalus[1].m_pnalu);
  BOOST_CHECK_EQUAL(psps -> sps_video_parameter_set_id, 11);
  BOOST_CHECK_EQUAL(psps -> sps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_id_nesting_flag, 1);

  BOOST_CHECK_EQUAL(psps -> pic_width_in_luma_samples, 1920);
  BOOST_CHECK_EQUAL(psps -> pic_height_in_luma_samples, 1080);

  BOOST_CHECK_EQUAL(psps -> sps_temporal_mvp_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> strong_intra_smoothing_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sps_extension_flag, 0);

  
  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_info.m_position, 0x000003f);
  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_pnalu -> getType(), NAL_PPS);

  std::shared_ptr<PPS> ppps = std::static_pointer_cast<PPS>(consumer.m_nalus[2].m_pnalu);
  BOOST_CHECK_EQUAL(ppps -> pps_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_seq_parameter_set_id, 2);
  BOOST_CHECK_EQUAL(ppps -> dependent_slice_segments_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> output_flag_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> num_extra_slice_header_bits, 0);
  BOOST_CHECK_EQUAL(ppps -> sign_data_hiding_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> deblocking_filter_control_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_scaling_list_data_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> lists_modification_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> log2_parallel_merge_level_minus2, 0);
  BOOST_CHECK_EQUAL(ppps -> slice_segment_header_extension_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_extension_flag, 0);
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_info.m_position, 0x4a);
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_pnalu -> getType(), NAL_IDR_W_RADL);
  std::shared_ptr<Slice> pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[3].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> no_output_of_prior_pics_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 2);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, 4);
  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 1);

  
  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_info.m_position, 0x67aa);
  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_pnalu -> getType(), NAL_TRAIL_R);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[4].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 1);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 2);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 0);

  BOOST_CHECK_EQUAL(pslice -> five_minus_max_num_merge_cand, 3);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, 4);
  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 1);


  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_info.m_position, 0xa1bb);
  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_pnalu -> getType(), NAL_TRAIL_N);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[5].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 0);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 0);

  BOOST_CHECK_EQUAL(pslice -> five_minus_max_num_merge_cand, 3);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, 4);
  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 1);
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_info.m_position, 0xaa71);
  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_pnalu -> getType(), NAL_TRAIL_R);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[6].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 1);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 3);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 0);

  BOOST_CHECK_EQUAL(pslice -> five_minus_max_num_merge_cand, 3);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, 4);
  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 1);
}


BOOST_AUTO_TEST_CASE(homer_default)
{
  Parser *pparser = Parser::create();
  
  Consumer consumer;
  
  pparser -> addConsumer(&consumer);
  
  std::ifstream in(getSourceDir() + "/samples/homer_default.hevc", std::ios::binary);
  
  in.seekg(0, std::ios::end);
  std::size_t size = in.tellg();
  in.seekg(0, std::ios::beg);
  
  char *pdata = new char[size];
  in.read(pdata, size);
  size = in.gcount();
  pparser -> process((const uint8_t *)pdata, size);
    
  pparser -> releaseConsumer(&consumer);
  Parser::release(pparser);
  

  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_info.m_position, 0x0);
  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_pnalu -> getType(), NAL_VPS);
  
  std::shared_ptr<VPS> pvps = std::static_pointer_cast<VPS>(consumer.m_nalus[0].m_pnalu);
  
  BOOST_CHECK_EQUAL(pvps -> vps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_temporal_id_nesting_flag, 1);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_level_idc, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_layer_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_num_layer_sets_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_timing_info_present_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_extension_flag, 0);
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_info.m_position, 0x1c);
  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_pnalu -> getType(), NAL_SPS);

  std::shared_ptr<SPS> psps = std::static_pointer_cast<SPS>(consumer.m_nalus[1].m_pnalu);
  BOOST_CHECK_EQUAL(psps -> sps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(psps -> sps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_id_nesting_flag, 1);

  BOOST_CHECK_EQUAL(psps -> pic_width_in_luma_samples, 1280);
  BOOST_CHECK_EQUAL(psps -> pic_height_in_luma_samples, 720);

  BOOST_CHECK_EQUAL(psps -> long_term_ref_pics_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_mvp_enabled_flag, 0);
  BOOST_CHECK_EQUAL(psps -> strong_intra_smoothing_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sps_extension_flag, 0);

  
  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_info.m_position, 0x0000040);
  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_pnalu -> getType(), NAL_PPS);

  std::shared_ptr<PPS> ppps = std::static_pointer_cast<PPS>(consumer.m_nalus[2].m_pnalu);
  BOOST_CHECK_EQUAL(ppps -> pps_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_seq_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> dependent_slice_segments_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> output_flag_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> num_extra_slice_header_bits, 0);
  BOOST_CHECK_EQUAL(ppps -> sign_data_hiding_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> deblocking_filter_control_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_scaling_list_data_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> lists_modification_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> log2_parallel_merge_level_minus2, 0);
  BOOST_CHECK_EQUAL(ppps -> slice_segment_header_extension_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_extension_flag, 0);
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_info.m_position, 0x4c);
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_pnalu -> getType(), NAL_IDR_W_RADL);
  std::shared_ptr<Slice> pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[3].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> no_output_of_prior_pics_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 2);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[0], 227);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[1], 181);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[2], 160);

  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_info.m_position, 0x3d07);
  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_pnalu -> getType(), NAL_TRAIL_R);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[4].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 1);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 1);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 1);

  BOOST_CHECK_EQUAL(pslice -> five_minus_max_num_merge_cand, 3);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[0], 101);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[1], 75);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[2], 77);


  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_info.m_position, 0x43b4);
  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_pnalu -> getType(), NAL_TRAIL_R);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[5].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 1);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 2);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 1);

  BOOST_CHECK_EQUAL(pslice -> five_minus_max_num_merge_cand, 3);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[0], 114);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[1], 82);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[2], 96);
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_info.m_position, 0x4baa);
  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_pnalu -> getType(), NAL_TRAIL_R);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[6].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 1);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 3);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 1);

  BOOST_CHECK_EQUAL(pslice -> five_minus_max_num_merge_cand, 3);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, 6);
  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[0], 140);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[1], 102);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[2], 83);
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[7].m_info.m_position, 0x53bd);
  BOOST_CHECK_EQUAL(consumer.m_nalus[7].m_pnalu -> getType(), NAL_TRAIL_R);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[7].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 1);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 4);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 1);

  BOOST_CHECK_EQUAL(pslice -> five_minus_max_num_merge_cand, 3);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[0], 149);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[1], 157);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[2], 117);
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[8].m_info.m_position, 0x5d32);
  BOOST_CHECK_EQUAL(consumer.m_nalus[8].m_pnalu -> getType(), NAL_TRAIL_R);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[8].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 1);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 5);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 1);

  BOOST_CHECK_EQUAL(pslice -> five_minus_max_num_merge_cand, 3);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[0], 141);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[1], 122);
  BOOST_CHECK_EQUAL(pslice -> entry_point_offset_minus1[2], 100);
  
}


BOOST_AUTO_TEST_CASE(BQSquare_416x240_60_qp37)
{
  Parser *pparser = Parser::create();
  
  Consumer consumer;
  
  pparser -> addConsumer(&consumer);
  
  std::ifstream in(getSourceDir() + "/samples/BQSquare_416x240_60_qp37.bin", std::ios::binary);
  
  in.seekg(0, std::ios::end);
  std::size_t size = in.tellg();
  in.seekg(0, std::ios::beg);
  
  char *pdata = new char[size];
  in.read(pdata, size);
  size = in.gcount();
  pparser -> process((const uint8_t *)pdata, size);
    
  pparser -> releaseConsumer(&consumer);
  Parser::release(pparser);
  

  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_info.m_position, 0x0);
  BOOST_CHECK_EQUAL(consumer.m_nalus[0].m_pnalu -> getType(), NAL_VPS);
  
  std::shared_ptr<VPS> pvps = std::static_pointer_cast<VPS>(consumer.m_nalus[0].m_pnalu);
  
  BOOST_CHECK_EQUAL(pvps -> vps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_temporal_id_nesting_flag, 1);
  BOOST_CHECK_EQUAL(pvps -> profile_tier_level.general_level_idc, 186);
  BOOST_CHECK_EQUAL(pvps -> vps_max_layer_id, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_num_layer_sets_minus1, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_timing_info_present_flag, 0);
  BOOST_CHECK_EQUAL(pvps -> vps_extension_flag, 0);
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_info.m_position, 0x1d);
  BOOST_CHECK_EQUAL(consumer.m_nalus[1].m_pnalu -> getType(), NAL_SPS);

  std::shared_ptr<SPS> psps = std::static_pointer_cast<SPS>(consumer.m_nalus[1].m_pnalu);
  BOOST_CHECK_EQUAL(psps -> sps_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(psps -> sps_max_sub_layers_minus1, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_id_nesting_flag, 1);

  BOOST_CHECK_EQUAL(psps -> pic_width_in_luma_samples, 416);
  BOOST_CHECK_EQUAL(psps -> pic_height_in_luma_samples, 240);

  BOOST_CHECK_EQUAL(psps -> long_term_ref_pics_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sps_temporal_mvp_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> strong_intra_smoothing_enabled_flag, 1);
  BOOST_CHECK_EQUAL(psps -> vui_parameters_present_flag, 0);
  BOOST_CHECK_EQUAL(psps -> sps_extension_flag, 0);

  
  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_info.m_position, 0x0000051);
  BOOST_CHECK_EQUAL(consumer.m_nalus[2].m_pnalu -> getType(), NAL_PPS);

  std::shared_ptr<PPS> ppps = std::static_pointer_cast<PPS>(consumer.m_nalus[2].m_pnalu);
  BOOST_CHECK_EQUAL(ppps -> pps_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_seq_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(ppps -> dependent_slice_segments_enabled_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> output_flag_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> num_extra_slice_header_bits, 0);
  BOOST_CHECK_EQUAL(ppps -> sign_data_hiding_flag, 1);
  BOOST_CHECK_EQUAL(ppps -> deblocking_filter_control_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_scaling_list_data_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> lists_modification_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> log2_parallel_merge_level_minus2, 0);
  BOOST_CHECK_EQUAL(ppps -> slice_segment_header_extension_present_flag, 0);
  BOOST_CHECK_EQUAL(ppps -> pps_extension_flag, 0);
  
  
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_info.m_position, 0x5c);
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_pnalu -> getType(), NAL_IDR_W_RADL);
  std::shared_ptr<Slice> pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[3].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> no_output_of_prior_pics_flag, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 2);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, 11);
  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 1);

  BOOST_CHECK_EQUAL(consumer.m_nalus[1201].m_info.m_position, 0x33a77);
  BOOST_CHECK_EQUAL(consumer.m_nalus[1201].m_pnalu -> getType(), NAL_TRAIL_N);
  pslice = std::static_pointer_cast<Slice>(consumer.m_nalus[1201].m_pnalu);

  BOOST_CHECK_EQUAL(pslice -> first_slice_segment_in_pic_flag, 1);
  BOOST_CHECK_EQUAL(pslice -> slice_pic_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_type, 0);
  BOOST_CHECK_EQUAL(pslice -> pic_order_cnt_lsb, 87);
  BOOST_CHECK_EQUAL(pslice -> short_term_ref_pic_set_sps_flag, 0);

  BOOST_CHECK_EQUAL(pslice -> five_minus_max_num_merge_cand, 0);
  BOOST_CHECK_EQUAL(pslice -> slice_qp_delta, 15);
  BOOST_CHECK_EQUAL(pslice -> slice_loop_filter_across_slices_enabled_flag, 1);


  BOOST_CHECK_EQUAL(consumer.m_nalus[1202].m_info.m_position, 0x33aab);
  BOOST_CHECK_EQUAL(consumer.m_nalus[1202].m_pnalu -> getType(), NAL_SEI_SUFFIX); 
  std::shared_ptr<SEI> psei  = std::static_pointer_cast<SEI>(consumer.m_nalus[1202].m_pnalu);

  BOOST_CHECK_EQUAL(psei -> sei_message.size(), 1);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_type_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_type_byte, 132);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_size_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_size_byte, 49);

  std::shared_ptr<DecodedPictureHash> pdecPictHash = std::static_pointer_cast<DecodedPictureHash>(psei -> sei_message[0].sei_payload);
  BOOST_CHECK_EQUAL(pdecPictHash->hash_type, 0);
  BOOST_CHECK_EQUAL(pdecPictHash->picture_md5.size(), 3);
  BOOST_CHECK_EQUAL(pdecPictHash->picture_md5.size(), 3);
  uint8_t hash[] = {0xA9, 0x0A, 0x8A, 0x08, 0x95, 0xE5, 0x1F, 0x9B, 0x79, 0x73, 0xA7, 0x91, 0xF9, 0x72, 0xF9, 0x07};
  BOOST_CHECK_EQUAL_COLLECTIONS(pdecPictHash->picture_md5[2].begin(), pdecPictHash->picture_md5[2].end(),
	hash, hash + sizeof(hash) / sizeof(hash[0]));
}


BOOST_AUTO_TEST_CASE(TestCase1_LifeOfPie_ReEncoded1080pX265_HDR_part)
{
  Parser *pparser = Parser::create();
  
  Consumer consumer;
  
  pparser -> addConsumer(&consumer);
  
  std::ifstream in(getSourceDir() + "/samples/TestCase1_LifeOfPie_ReEncoded1080pX265_HDR_part.hevc", std::ios::binary);
  
  in.seekg(0, std::ios::end);
  std::size_t size = in.tellg();
  in.seekg(0, std::ios::beg);
  
  char *pdata = new char[size];
  in.read(pdata, size);
  size = in.gcount();
  pparser -> process((const uint8_t *)pdata, size);
    
  pparser -> releaseConsumer(&consumer);
  Parser::release(pparser);
  

  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_info.m_position, 0x65);
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_pnalu -> getType(), NAL_SEI_PREFIX);
  
  std::shared_ptr<SEI> psei = std::static_pointer_cast<SEI>(consumer.m_nalus[3].m_pnalu);
  
  BOOST_CHECK_EQUAL(psei -> sei_message.size(), 1);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_type_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_type_byte, 137);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_size_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_size_byte, 24);

  std::shared_ptr<MasteringDisplayInfo> pmdi = std::static_pointer_cast<MasteringDisplayInfo>(psei -> sei_message[0].sei_payload);

  BOOST_CHECK_EQUAL(pmdi -> display_primary_x[0], 13250);
  BOOST_CHECK_EQUAL(pmdi -> display_primary_x[1], 7500);
  BOOST_CHECK_EQUAL(pmdi -> display_primary_x[2], 34000);
  BOOST_CHECK_EQUAL(pmdi -> display_primary_y[0], 34500);
  BOOST_CHECK_EQUAL(pmdi -> display_primary_y[1], 3000);
  BOOST_CHECK_EQUAL(pmdi -> display_primary_y[2], 16000);
  BOOST_CHECK_EQUAL(pmdi -> white_point_x, 15635);
  BOOST_CHECK_EQUAL(pmdi -> white_point_y, 16450);
  BOOST_CHECK_EQUAL(pmdi -> max_display_mastering_luminance, 12000000);
  BOOST_CHECK_EQUAL(pmdi -> min_display_mastering_luminance, 200);


  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_info.m_position, 0x87);
  BOOST_CHECK_EQUAL(consumer.m_nalus[4].m_pnalu -> getType(), NAL_SEI_PREFIX);
  
  psei = std::static_pointer_cast<SEI>(consumer.m_nalus[4].m_pnalu);
  
  BOOST_CHECK_EQUAL(psei -> sei_message.size(), 1);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_type_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_type_byte, 5);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_size_ff_bytes, 3);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_size_byte, 237);

  std::shared_ptr<UserDataUnregistered> pudu = std::static_pointer_cast<UserDataUnregistered>(psei -> sei_message[0].sei_payload);


  uint8_t uuid_iso_iec_11578[] = {0x2c, 0xa2, 0xde, 0x09, 0xb5, 0x17, 0x47, 0xdb, 0xbb, 0x55, 0xa4, 0xfe, 0x7f, 0xc2, 0xfc, 0x4e};

  BOOST_CHECK_EQUAL_COLLECTIONS(uuid_iso_iec_11578, uuid_iso_iec_11578 + 16, 
    pudu->uuid_iso_iec_11578, pudu->uuid_iso_iec_11578 + 16);


  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_info.m_position, 0x47d);
  BOOST_CHECK_EQUAL(consumer.m_nalus[5].m_pnalu -> getType(), NAL_SEI_PREFIX);

  psei = std::static_pointer_cast<SEI>(consumer.m_nalus[5].m_pnalu);
  
  BOOST_CHECK_EQUAL(psei -> sei_message.size(), 1);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_type_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_type_byte, 129);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_size_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_size_byte, 1);

  std::shared_ptr<ActiveParameterSets> paps = std::static_pointer_cast<ActiveParameterSets>(psei -> sei_message[0].sei_payload);

  BOOST_CHECK_EQUAL(paps -> active_video_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(paps -> self_contained_cvs_flag, 1);
  BOOST_CHECK_EQUAL(paps -> no_parameter_set_update_flag, 1);
  BOOST_CHECK_EQUAL(paps -> num_sps_ids_minus1, 0);
  BOOST_CHECK_EQUAL(paps -> active_seq_parameter_set_id.size(), 1);
  BOOST_CHECK_EQUAL(paps -> active_seq_parameter_set_id[0], 0);



  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_info.m_position, 0x487);
  BOOST_CHECK_EQUAL(consumer.m_nalus[6].m_pnalu -> getType(), NAL_SEI_PREFIX);

  psei = std::static_pointer_cast<SEI>(consumer.m_nalus[6].m_pnalu);
  
  BOOST_CHECK_EQUAL(psei -> sei_message.size(), 1);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_type_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_type_byte, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_size_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_size_byte, 7);

  std::shared_ptr<BufferingPeriod> pbp = std::static_pointer_cast<BufferingPeriod>(psei -> sei_message[0].sei_payload);

  BOOST_CHECK_EQUAL(pbp -> bp_seq_parameter_set_id, 0);
  BOOST_CHECK_EQUAL(pbp -> irap_cpb_params_present_flag, 0);
  BOOST_CHECK_EQUAL(pbp -> concatenation_flag, 0);
  BOOST_CHECK_EQUAL(pbp -> au_cpb_removal_delay_delta_minus1, 0);
  BOOST_CHECK_EQUAL(pbp -> nal_initial_cpb_removal_delay.size(), 1);
  BOOST_CHECK_EQUAL(pbp -> nal_initial_cpb_removal_delay[0], 81001);
  BOOST_CHECK_EQUAL(pbp -> nal_initial_cpb_removal_offset.size(), 1);
  BOOST_CHECK_EQUAL(pbp -> nal_initial_cpb_removal_offset[0], 9000);
  BOOST_CHECK_EQUAL(pbp -> vcl_initial_cpb_removal_delay.size(), 0);
  BOOST_CHECK_EQUAL(pbp -> vcl_initial_cpb_removal_offset.size(), 0);
  BOOST_CHECK_EQUAL(pbp -> vcl_initial_alt_cpb_removal_delay.size(), 0);
  BOOST_CHECK_EQUAL(pbp -> vcl_initial_alt_cpb_removal_offset.size(), 0);


  BOOST_CHECK_EQUAL(consumer.m_nalus[7].m_info.m_position, 0x497);
  BOOST_CHECK_EQUAL(consumer.m_nalus[7].m_pnalu -> getType(), NAL_SEI_PREFIX);

  psei = std::static_pointer_cast<SEI>(consumer.m_nalus[7].m_pnalu);
  
  BOOST_CHECK_EQUAL(psei -> sei_message.size(), 1);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_type_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_type_byte, 1);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_size_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_size_byte, 3);

  std::shared_ptr<PicTiming> ppt = std::static_pointer_cast<PicTiming>(psei -> sei_message[0].sei_payload);
  BOOST_CHECK_EQUAL(ppt -> au_cpb_removal_delay_minus1, 0);
  BOOST_CHECK_EQUAL(ppt -> pic_dpb_output_delay, 2);


  BOOST_CHECK_EQUAL(consumer.m_nalus[13].m_info.m_position, 0xb85d);
  BOOST_CHECK_EQUAL(consumer.m_nalus[13].m_pnalu -> getType(), NAL_SEI_PREFIX);

  psei = std::static_pointer_cast<SEI>(consumer.m_nalus[13].m_pnalu);
  
  BOOST_CHECK_EQUAL(psei -> sei_message.size(), 1);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_type_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_type_byte, 1);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_size_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_size_byte, 3);

  ppt = std::static_pointer_cast<PicTiming>(psei -> sei_message[0].sei_payload);
  BOOST_CHECK_EQUAL(ppt -> au_cpb_removal_delay_minus1, 0);
  BOOST_CHECK_EQUAL(ppt -> pic_dpb_output_delay, 5);


  BOOST_CHECK_EQUAL(consumer.m_nalus[15].m_info.m_position, 0x14784);
  BOOST_CHECK_EQUAL(consumer.m_nalus[15].m_pnalu -> getType(), NAL_SEI_PREFIX);

  psei = std::static_pointer_cast<SEI>(consumer.m_nalus[15].m_pnalu);
  
  BOOST_CHECK_EQUAL(psei -> sei_message.size(), 1);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_type_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_type_byte, 1);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_size_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_size_byte, 3);

  ppt = std::static_pointer_cast<PicTiming>(psei -> sei_message[0].sei_payload);
  BOOST_CHECK_EQUAL(ppt -> au_cpb_removal_delay_minus1, 1);
  BOOST_CHECK_EQUAL(ppt -> pic_dpb_output_delay, 2);
}


BOOST_AUTO_TEST_CASE(x265_cll)
{
  Parser *pparser = Parser::create();
  
  Consumer consumer;
  
  pparser -> addConsumer(&consumer);
  
  std::ifstream in(getSourceDir() + "/samples/x265_cll.hevc", std::ios::binary);
  
  in.seekg(0, std::ios::end);
  std::size_t size = in.tellg();
  in.seekg(0, std::ios::beg);
  
  char *pdata = new char[size];
  in.read(pdata, size);
  size = in.gcount();
  pparser -> process((const uint8_t *)pdata, size);
    
  pparser -> releaseConsumer(&consumer);
  Parser::release(pparser);
  

  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_info.m_position, 0x53);
  BOOST_CHECK_EQUAL(consumer.m_nalus[3].m_pnalu -> getType(), NAL_SEI_PREFIX);
  
  std::shared_ptr<SEI> psei = std::static_pointer_cast<SEI>(consumer.m_nalus[3].m_pnalu);
  
  BOOST_CHECK_EQUAL(psei -> sei_message.size(), 1);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_type_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_type_byte, 144);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].num_payload_size_ff_bytes, 0);
  BOOST_CHECK_EQUAL(psei -> sei_message[0].last_payload_size_byte, 4);

  std::shared_ptr<ContentLightLevelInfo> pcll = std::static_pointer_cast<ContentLightLevelInfo>(psei -> sei_message[0].sei_payload);

  BOOST_CHECK_EQUAL(pcll -> max_content_light_level, 16000);
  BOOST_CHECK_EQUAL(pcll -> max_pic_average_light_level, 32000);
}


BOOST_AUTO_TEST_SUITE_END();
