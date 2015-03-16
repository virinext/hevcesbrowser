#include "HevcParserImpl.h"
#include "HevcUtils.h"

#include <iostream>
#include <stdexcept>

using namespace HEVC;

void HevcParserImpl::addConsumer(Consumer *pconsumer)
{
  m_consumers.push_back(pconsumer);
}


void HevcParserImpl::releaseConsumer(Consumer *pconsumer)
{
  m_consumers.remove(pconsumer);
}

std::size_t HevcParserImpl::process(const uint8_t *pdata, std::size_t size, std::size_t offset)
{
  std::size_t parsed = 0;
  bool parseFailed = false;
  for(std::size_t pos = 0; pos + 3 < size;)
  {
    std::size_t startOffset = 3;
    bool naluFinded = pdata[pos] == 0 && pdata[pos+1] == 0 && pdata[pos+2] == 1;

    if(!naluFinded)
    {
      if(pos - size >= 4 &&pdata[pos] == 0 && pdata[pos+1] == 0 && pdata[pos+2] == 0 && pdata[pos+3] == 1)
      {
        naluFinded = true;
        startOffset = 4;
      }
    }
    if(naluFinded)
    {
      Parser::Info info;
      info.m_position = offset + pos;

      try
      {
        processNALUnit(pdata + pos + startOffset, size - pos - startOffset, info);
      }
      catch (std::runtime_error &err)
      {
        std::cerr << pos << " " << err.what();
        parseFailed = true;
        break;
      }
      parsed = pos;
      pos += 3;
    }
     else
      pos++;
  }

  if(!parseFailed)
  {
    parsed = size;

    for(std::size_t i=0; i<3; i++)
    {
      if(pdata[size - i - 1] == 0)
        parsed--;
      else 
        break;
    }
  }

  return parsed;
}


void HevcParserImpl::processNALUnit(const uint8_t *pdata, std::size_t size, const Parser::Info &info)
{
  BitstreamReader bs(pdata, size);

  NALUnitType type = processNALUnitHeader(bs);

  std::shared_ptr<NALUnit> pnalU;

  switch(type)
  {
    case NAL_VPS:
    {
      std::shared_ptr<VPS> pvps(new VPS);
      processVPS(pvps, bs, info);
      pnalU = pvps;
      m_vpsMap[pvps -> vps_video_parameter_set_id] = pvps;
      break;
    }

    case NAL_SPS:
    {
      std::shared_ptr<SPS> psps(new SPS);
      processSPS(psps, bs, info);
      pnalU = psps;
      m_spsMap[psps -> sps_seq_parameter_set_id] = psps;
      break;
    }

    case NAL_PPS:
    {
      std::shared_ptr<PPS> ppps(new PPS);
      processPPS(ppps, bs, info);
      pnalU = ppps;
      m_ppsMap[ppps -> pps_pic_parameter_set_id] = ppps;
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
      std::shared_ptr<Slice> pslice(new Slice(type));
      processSlice(pslice, bs, info);
      pnalU = pslice;
      break;
    }

    default:
      pnalU = std::shared_ptr<NALUnit>(new NALUnit(type));
  };

  std::list<Consumer *>::const_iterator itr = m_consumers.begin();

  for(; itr != m_consumers.end(); itr++)
    (*itr) -> onNALUnit(pnalU, &info);
}


NALUnitType HevcParserImpl::processNALUnitHeader(BitstreamReader &bs)
{
  //forbidden_zero_bit
  bs.getBit();

  NALUnitType type = (NALUnitType)bs.getBits(6);

  //nuh_layer_id
  bs.getBits(6);

  //nuh_temporal_id_plus1
  bs.getBits(3);

  return type;
}



void HevcParserImpl::processSlice(std::shared_ptr<Slice> pslice, BitstreamReader &bs, const Parser::Info &info)
{
  processSliceHeader(pslice, bs, info);
  processSliceData(pslice, bs, info);
}


void HevcParserImpl::processSliceHeader(std::shared_ptr<Slice> pslice, BitstreamReader &bs, const Parser::Info &info)
{
  pslice -> first_slice_segment_in_pic_flag = bs.getBits(1);

  if(pslice -> m_nalUnitType >= NAL_BLA_W_LP && pslice -> m_nalUnitType <= NAL_IRAP_VCL23)
    pslice -> no_output_of_prior_pics_flag = bs.getBits(1);

  pslice -> slice_pic_parameter_set_id = bs.getGolombU();

  std::shared_ptr<PPS> ppps = m_ppsMap[pslice -> slice_pic_parameter_set_id];
  int32_t spsId = ppps -> pps_seq_parameter_set_id;

  pslice -> dependent_slice_segment_flag = 0;
  if(!pslice -> first_slice_segment_in_pic_flag)
  {
    if(ppps -> dependent_slice_segments_enabled_flag)
      pslice -> dependent_slice_segment_flag = bs.getBits(1);
    else
      pslice -> dependent_slice_segment_flag = 0;

    int32_t sliceAddrLength = HEVC::log2(
    	(m_spsMap[spsId] -> pic_width_in_luma_samples * m_spsMap[spsId] -> pic_height_in_luma_samples - 1) << 1);
      pslice -> slice_segment_address = bs.getBits(sliceAddrLength);
  }

  if(!pslice -> dependent_slice_segment_flag)
  {
    uint32_t num_extra_slice_header_bits = ppps -> num_extra_slice_header_bits;
    pslice -> slice_reserved_undetermined_flag.resize(num_extra_slice_header_bits, 0);
    for(std::size_t i=0; i<num_extra_slice_header_bits; i++)
      pslice -> slice_reserved_undetermined_flag[i] = bs.getBits(1);

    pslice -> slice_type = bs.getGolombU();

    if(ppps -> output_flag_present_flag)
      pslice -> pic_output_flag = bs.getBits(1);

    if(m_spsMap[spsId] -> separate_colour_plane_flag)
      pslice -> colour_plane_id = bs.getBits(2);

    bool IdrPicFlag = pslice -> m_nalUnitType == NAL_IDR_W_RADL || pslice -> m_nalUnitType == NAL_IDR_N_LP;
    if(!IdrPicFlag)
    {
      pslice -> pic_order_cnt_lsb = bs.getBits(m_spsMap[spsId] -> log2_max_pic_order_cnt_lsb_minus4 + 4);
      pslice -> short_term_ref_pic_set_sps_flag = bs.getBits(1);

      if(!pslice -> short_term_ref_pic_set_sps_flag)
      {
//      throw std::runtime_error(std::string(__FILE__) + " (" +
//        std::to_string(__LINE__) + "): short_term_ref_pic_set is not implemented");
      }
      else if(pslice -> short_term_ref_pic_set_sps_flag > 1)
      {
        std::size_t numBits = HEVC::log2(m_spsMap[spsId] -> num_short_term_ref_pic_sets);
        if(numBits > 0)
          pslice -> short_term_ref_pic_set_idx = bs.getBits(numBits);
        else
          pslice -> short_term_ref_pic_set_idx = 0;
      }
    }
  }
}


void HevcParserImpl::processSliceData(std::shared_ptr<Slice> pslice, BitstreamReader &bs, const Parser::Info &info)
{
}


void HevcParserImpl::processVPS(std::shared_ptr<VPS> pvps, BitstreamReader &bs, const Parser::Info &info)
{
  pvps -> vps_video_parameter_set_id = bs.getBits(4);
  bs.getBits(2);
  pvps -> vps_max_layers_minus1 = bs.getBits(6);
  pvps -> vps_max_sub_layers_minus1 = bs.getBits(3);
  pvps -> vps_temporal_id_nesting_flag = bs.getBits(1);
}


void HevcParserImpl::processSPS(std::shared_ptr<SPS> psps, BitstreamReader &bs, const Parser::Info &info)
{
  psps -> sps_video_parameter_set_id = bs.getBits(4);
  psps -> sps_max_sub_layers_minus1 = bs.getBits(3);
  psps -> sps_temporal_id_nesting_flag = bs.getBits(1);
  processProfileTierLevel(psps, bs, info);

  psps -> sps_seq_parameter_set_id = bs.getGolombU();
  psps -> sps_seq_parameter_set_id = 0;
  psps -> chroma_format_idc = bs.getGolombU();

  if(psps -> chroma_format_idc == 3)
    psps -> separate_colour_plane_flag = bs.getBits(1);
  else
    psps -> separate_colour_plane_flag = 0;

  psps -> pic_width_in_luma_samples = bs.getGolombU();
  psps -> pic_height_in_luma_samples = bs.getGolombU();
  psps -> conformance_window_flag = bs.getBits(1);

  if(psps -> conformance_window_flag)
  {
    psps -> conf_win_left_offset = bs.getGolombU();
    psps -> conf_win_right_offset = bs.getGolombU();
    psps -> conf_win_top_offset = bs.getGolombU();
    psps -> conf_win_bottom_offset = bs.getGolombU();
  }

  psps -> bit_depth_luma_minus8 = bs.getGolombU();
  psps -> bit_depth_chroma_minus8 = bs.getGolombU();
  psps -> log2_max_pic_order_cnt_lsb_minus4 = bs.getGolombU();
  psps -> sps_sub_layer_ordering_info_present_flag = bs.getBits(1);

  psps -> sps_max_dec_pic_buffering_minus1.resize(psps -> sps_max_sub_layers_minus1 + 1, 0);
  psps -> sps_max_num_reorder_pics.resize(psps -> sps_max_sub_layers_minus1 + 1, 0);
  psps -> sps_max_latency_increase.resize(psps -> sps_max_sub_layers_minus1 + 1, 0);

  for(std::size_t i=(psps -> sps_sub_layer_ordering_info_present_flag ? 0 : psps -> sps_max_sub_layers_minus1);
      i<=psps -> sps_max_sub_layers_minus1;
      i++)
  {
    psps -> sps_max_dec_pic_buffering_minus1[i] = bs.getGolombU();
    psps -> sps_max_num_reorder_pics[i] = bs.getGolombU();
    psps -> sps_max_latency_increase[i] = bs.getGolombU();
  }

  psps -> log2_min_luma_coding_block_size_minus3 = bs.getGolombU();
  psps -> log2_diff_max_min_luma_coding_block_size = bs.getGolombU();
  psps -> log2_min_transform_block_size_minus2 = bs.getGolombU();
  psps -> log2_diff_max_min_transform_block_size = bs.getGolombU();
  psps -> max_transform_hierarchy_depth_inter = bs.getGolombU();
  psps -> max_transform_hierarchy_depth_intra = bs.getGolombU();

  psps -> scaling_list_enabled_flag = bs.getBits(1);
  if(psps -> scaling_list_enabled_flag)
  {
    psps -> sps_scaling_list_data_present_flag = bs.getBits(1);
    if(psps -> sps_scaling_list_data_present_flag)
    {
      throw std::runtime_error(std::string(__FILE__) + " (" +
	std::to_string(__LINE__) + "): sps_scaling_list is not implemented");
    }
  }

  psps -> amp_enabled_flag = bs.getBits(1);
  psps -> sample_adaptive_offset_enabled_flag = bs.getBits(1);
  psps -> pcm_enabled_flag = bs.getBits(1);

  if(psps -> pcm_enabled_flag)
  {
    throw std::runtime_error(std::string(__FILE__) + " (" +
      std::to_string(__LINE__) + "): pcm params support is not implemented");
  }

  psps -> num_short_term_ref_pic_sets = bs.getGolombU();

}



void HevcParserImpl::processProfileTierLevel(std::shared_ptr<SPS> psps, BitstreamReader &bs, const Parser::Info &info)
{
  psps -> profile_tier_level.general_profile_space = bs.getBits(2);
  psps -> profile_tier_level.general_tier_flag = bs.getBits(1);
  psps -> profile_tier_level.general_profile_idc = bs.getBits(5);

  for(std::size_t i=0; i<32; i++)
    psps -> profile_tier_level.general_profile_compatibility_flag[i] = bs.getBits(1);

  psps -> profile_tier_level.general_progressive_source_flag = bs.getBits(1);
  psps -> profile_tier_level.general_interlaced_source_flag = bs.getBits(1);
  psps -> profile_tier_level.general_non_packed_constraint_flag = bs.getBits(1);
  psps -> profile_tier_level.general_frame_only_constraint_flag = bs.getBits(1);
  bs.getBits(32);
  bs.getBits(12);
  psps -> profile_tier_level.general_level_idc = bs.getBits(8);

  psps -> profile_tier_level.sub_layer_profile_present_flag.resize(psps -> sps_max_sub_layers_minus1);
  psps -> profile_tier_level.sub_layer_level_present_flag.resize(psps -> sps_max_sub_layers_minus1);

  for(std::size_t i=0; i<psps -> sps_max_sub_layers_minus1; i++)
  {
    psps -> profile_tier_level.sub_layer_profile_present_flag[i] = bs.getBits(1);
    psps -> profile_tier_level.sub_layer_level_present_flag[i] = bs.getBits(1);
  }


  if(psps -> sps_max_sub_layers_minus1 > 0)
  {
    for(std::size_t i=psps -> sps_max_sub_layers_minus1; i<8; i++)
      bs.getBits(2);
  }

  for(std::size_t i=0; i<psps -> sps_max_sub_layers_minus1; i++)
  {
    if(psps -> profile_tier_level.sub_layer_profile_present_flag[i])
    {
      bs.getBits(32);
      bs.getBits(32);
      bs.getBits(24);
    }

    if(psps -> profile_tier_level.sub_layer_level_present_flag[i])
    {
      bs.getBits(8);
    }
  }
}


void HevcParserImpl::processPPS(std::shared_ptr<PPS> ppps, BitstreamReader &bs, const Parser::Info &info)
{
  ppps -> pps_pic_parameter_set_id = bs.getGolombU();
  ppps -> pps_seq_parameter_set_id  = bs.getGolombU();
  ppps -> dependent_slice_segments_enabled_flag = bs.getBits(1);

  ppps -> output_flag_present_flag = bs.getBits(1);
  ppps -> num_extra_slice_header_bits = bs.getBits(3);
  ppps -> sign_data_hiding_flag = bs.getBits(1);
  ppps -> cabac_init_present_flag = bs.getBits(1);
  ppps -> num_ref_idx_l0_default_active_minus1 = bs.getGolombU();
  ppps -> num_ref_idx_l1_default_active_minus1 = bs.getGolombU();
  ppps -> init_qp_minus26  = bs.getGolombS();
  ppps -> constrained_intra_pred_flag = bs.getBits(1);
  ppps -> transform_skip_enabled_flag = bs.getBits(1);
  ppps -> cu_qp_delta_enabled_flag = bs.getBits(1);

  if(ppps -> cu_qp_delta_enabled_flag)
    ppps -> diff_cu_qp_delta_depth = bs.getGolombU();

  ppps -> pps_cb_qp_offset = bs.getGolombS();
  ppps -> pps_cr_qp_offset = bs.getGolombS();
  ppps -> pps_slice_chroma_qp_offsets_present_flag = bs.getBits(1);
  ppps -> weighted_pred_flag = bs.getBits(1);
  ppps -> weighted_bipred_flag = bs.getBits(1);
  ppps -> transquant_bypass_enabled_flag = bs.getBits(1);
  ppps -> tiles_enabled_flag = bs.getBits(1);
  ppps -> entropy_coding_sync_enabled_flag = bs.getBits(1);
}
