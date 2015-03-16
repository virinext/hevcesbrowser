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
      nalu.m_pnalu = std::move(pNALUnit -> copy());
      nalu.m_info = *pInfo;
      
      m_nalus.push_back(nalu);
    }
        
    std::vector<NALUInfo>    m_nalus;
};

BOOST_AUTO_TEST_CASE(SURFING_FIRST_30)
{
  Parser *pparser = Parser::create();
  
  Consumer consumer;
  
  pparser -> addConsumer(&consumer);
  
  std::ifstream in(getSourceDir() + "/samples/surfing.265", std::ios::binary);
  
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
  BOOST_CHECK_EQUAL(psps -> sps_max_latency_increase[0], 0);

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
  
  std::ifstream in(getSourceDir() + "/samples/Sintel_272p_logo.265", std::ios::binary);
  
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
  BOOST_CHECK_EQUAL(psps -> sps_max_latency_increase[0], 0);
  BOOST_CHECK_EQUAL(psps -> sps_max_dec_pic_buffering_minus1[1], 2);
  BOOST_CHECK_EQUAL(psps -> sps_max_num_reorder_pics[1], 1);
  BOOST_CHECK_EQUAL(psps -> sps_max_latency_increase[1], 0);
  BOOST_CHECK_EQUAL(psps -> sps_max_dec_pic_buffering_minus1[2], 3);
  BOOST_CHECK_EQUAL(psps -> sps_max_num_reorder_pics[2], 2);
  BOOST_CHECK_EQUAL(psps -> sps_max_latency_increase[2], 0);

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



BOOST_AUTO_TEST_SUITE_END();
