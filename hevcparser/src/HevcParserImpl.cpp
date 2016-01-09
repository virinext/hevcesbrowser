#include "HevcParserImpl.h"
#include "HevcUtils.h"

#include <iostream>
#include <stdexcept>
#include <string>

#include <sstream>

#include <assert.h>

using namespace HEVC;

#define SLICE_B 0
#define SLICE_P 1
#define SLICE_I 2

void HevcParserImpl::addConsumer(Consumer *pconsumer)
{
  m_consumers.push_back(pconsumer);
}


void HevcParserImpl::releaseConsumer(Consumer *pconsumer)
{
  m_consumers.remove(pconsumer);
}


void HevcParserImpl::onWarning(const std::string &warning, const Info *pInfo, WarningType type)
{
  std::list<Consumer *>::const_iterator itr = m_consumers.begin();
  for(; itr != m_consumers.end(); itr++)
    (*itr) -> onWarning(warning, pInfo, type);
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
      if(size - pos >= 4 &&pdata[pos] == 0 && pdata[pos+1] == 0 && pdata[pos+2] == 0 && pdata[pos+3] == 1)
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
      catch (std::bad_alloc &err)
      {
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

    case NAL_AUD:
    {
      std::shared_ptr<AUD> paud(new AUD);
      processAUD(paud, bs, info);
      pnalU = paud;
      break;
    }

    case NAL_SEI_SUFFIX:
    case NAL_SEI_PREFIX:
    {
      std::shared_ptr<SEI> psei(new SEI(type));
      processSEI(psei, bs, info);
      pnalU = psei;
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
  if(!ppps)
  {
    std::stringstream ss;
    ss << "Slice: slice_pic_parameter_set_id = " 
      << (int) pslice -> slice_pic_parameter_set_id
      << ", but PPS with this id not exists";
    
    onWarning(ss.str(), &info, Parser::REFERENCE_STRUCT_NOT_PRESENT);

    pslice -> m_processFailed = true;

    return;
  }

  int32_t spsId = ppps -> pps_seq_parameter_set_id;

  if(m_spsMap.find(spsId) == m_spsMap.end())
  {
    std::stringstream ss;
    ss << "Slice: pps_seq_parameter_set_id = "
      << (int) ppps -> pps_seq_parameter_set_id
      << ", but SPS with this id not exists";

    onWarning(ss.str(), &info, Parser::REFERENCE_STRUCT_NOT_PRESENT);

    pslice -> m_processFailed = true;

    return;
  }

  pslice -> dependent_slice_segment_flag = 0;
  if(!pslice -> first_slice_segment_in_pic_flag)
  {
    if(ppps -> dependent_slice_segments_enabled_flag)
      pslice -> dependent_slice_segment_flag = bs.getBits(1);
    else
      pslice -> dependent_slice_segment_flag = 0;
    int32_t CtbLog2SizeY = m_spsMap[spsId] -> log2_min_luma_coding_block_size_minus3 + 3 + m_spsMap[spsId] -> log2_diff_max_min_luma_coding_block_size;
    uint32_t CtbSizeY = 1 << CtbLog2SizeY;
    uint32_t PicWidthInCtbsY = m_spsMap[spsId] -> pic_width_in_luma_samples / CtbSizeY;
    if(m_spsMap[spsId] -> pic_width_in_luma_samples % CtbSizeY)
      PicWidthInCtbsY++;

    uint32_t PicHeightInCtbsY = m_spsMap[spsId] -> pic_height_in_luma_samples / CtbSizeY;
    if(m_spsMap[spsId] -> pic_height_in_luma_samples % CtbSizeY)
      PicHeightInCtbsY++;

    int32_t sliceAddrLength = HEVC::log2(PicHeightInCtbsY * PicWidthInCtbsY);
    if((1 << sliceAddrLength) < PicHeightInCtbsY * PicWidthInCtbsY)
      sliceAddrLength++;

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
      if(m_spsMap[spsId] -> log2_max_pic_order_cnt_lsb_minus4 + 4 >= 32)
      {
        onWarning("Slice: pic_order_cnt_lsb size more then 32 bits", &info, Parser::OUT_OF_RANGE);

        pslice -> m_processFailed = true;

        return;
      }

      pslice -> pic_order_cnt_lsb = bs.getBits(m_spsMap[spsId] -> log2_max_pic_order_cnt_lsb_minus4 + 4);
      pslice -> short_term_ref_pic_set_sps_flag = bs.getBits(1);

      if(!pslice -> short_term_ref_pic_set_sps_flag)
      {
        pslice -> short_term_ref_pic_set = processShortTermRefPicSet(m_spsMap[spsId] -> num_short_term_ref_pic_sets, m_spsMap[spsId] -> num_short_term_ref_pic_sets, m_spsMap[spsId] -> short_term_ref_pic_set, m_spsMap[spsId], bs, info);
      }
      else if(m_spsMap[spsId] -> num_short_term_ref_pic_sets > 1)
      {
        std::size_t numBits = HEVC::log2(m_spsMap[spsId] -> num_short_term_ref_pic_sets);
        if(1 << numBits < m_spsMap[spsId] -> num_short_term_ref_pic_sets)
          numBits++;

        if(numBits > 0)
          pslice -> short_term_ref_pic_set_idx = bs.getBits(numBits);
        else
          pslice -> short_term_ref_pic_set_idx = 0;
      }

      if(m_spsMap[spsId] -> long_term_ref_pics_present_flag)
      {
        pslice -> num_long_term_sps = 0;
        if(m_spsMap[spsId] -> num_long_term_ref_pics_sps > 0)
          pslice -> num_long_term_sps = bs.getGolombU();

        pslice -> num_long_term_pics = bs.getGolombU();

        std::size_t num_long_term = pslice -> num_long_term_sps + pslice -> num_long_term_pics;

        pslice -> lt_idx_sps.resize(num_long_term);
        pslice -> poc_lsb_lt.resize(num_long_term);
        pslice -> used_by_curr_pic_lt_flag.resize(num_long_term);
        pslice -> delta_poc_msb_present_flag.resize(num_long_term);
        pslice -> delta_poc_msb_cycle_lt.resize(num_long_term);

        for(std::size_t i=0; i < num_long_term; i++)
        {
          if(i < pslice -> num_long_term_sps) 
          {
            if(m_spsMap[spsId] -> num_long_term_ref_pics_sps > 1)
            {
              int32_t ltIdxSpsLength = HEVC::log2(m_spsMap[spsId] -> num_long_term_ref_pics_sps);
              pslice -> lt_idx_sps[i] = bs.getBits(ltIdxSpsLength);
            }
          }
          else
          {
            pslice -> poc_lsb_lt[i] = bs.getBits(m_spsMap[spsId] -> log2_max_pic_order_cnt_lsb_minus4 + 4);
            pslice -> used_by_curr_pic_lt_flag[i] = bs.getBits(1);
          }

          pslice -> delta_poc_msb_present_flag[i] = bs.getBits(1);
          if(pslice -> delta_poc_msb_present_flag[i])
            pslice -> delta_poc_msb_cycle_lt[i] = bs.getGolombU();

        }
      }

      if(m_spsMap[spsId] -> sps_temporal_mvp_enabled_flag)
        pslice -> slice_temporal_mvp_enabled_flag = bs.getBits(1);
    }

    if(m_spsMap[spsId] -> sample_adaptive_offset_enabled_flag)
    {
      pslice -> slice_sao_luma_flag = bs.getBits(1);
      pslice -> slice_sao_chroma_flag = bs.getBits(1);
    }

    pslice -> num_ref_idx_l1_active_minus1 = ppps -> num_ref_idx_l0_default_active_minus1;
    pslice -> num_ref_idx_l0_active_minus1 = ppps -> num_ref_idx_l1_default_active_minus1;

    if(pslice -> slice_type == SLICE_B || pslice -> slice_type == SLICE_P)
    {
      pslice -> num_ref_idx_active_override_flag = bs.getBits(1);
      if(pslice -> num_ref_idx_active_override_flag)
      {
        pslice -> num_ref_idx_l0_active_minus1 = bs.getGolombU();

        if(pslice -> slice_type == SLICE_B)
          pslice -> num_ref_idx_l1_active_minus1 = bs.getGolombU();
      }

      if(ppps -> lists_modification_present_flag)
      {
        std::size_t NumPocTotalCurr = calcNumPocTotalCurr(pslice, m_spsMap[spsId]);
        if(NumPocTotalCurr > 1)
          pslice -> ref_pic_lists_modification = processRefPicListModification(bs, pslice);
      }

      if(pslice -> slice_type == SLICE_B)
        pslice -> mvd_l1_zero_flag = bs.getBits(1);

      if(ppps -> cabac_init_present_flag)
        pslice -> cabac_init_flag = bs.getBits(1);

      if(pslice -> slice_temporal_mvp_enabled_flag)
      {
        if(pslice -> slice_type == SLICE_B)
          pslice -> collocated_from_l0_flag = bs.getBits(1);

        if(pslice -> collocated_from_l0_flag && pslice -> num_ref_idx_l0_active_minus1 ||
            !pslice -> collocated_from_l0_flag && pslice -> num_ref_idx_l1_active_minus1)
        {
          pslice -> collocated_ref_idx = bs.getGolombU();
        }
      }

      if(ppps -> weighted_pred_flag && pslice -> slice_type == SLICE_B ||
        ppps -> weighted_bipred_flag && pslice -> slice_type == SLICE_P)
      {
        pslice -> pred_weight_table = processPredWeightTable(bs, pslice);

        if(pslice -> pred_weight_table.luma_log2_weight_denom > 7)
        {
          std::stringstream ss;
          ss << "pred_weight_table.luma_log2_weight_denom = " 
            << (int) pslice -> pred_weight_table.luma_log2_weight_denom 
            << ", but must be in range (0-7)";
          
          onWarning(ss.str(), &info, Parser::OUT_OF_RANGE);
        }
      }

      pslice -> five_minus_max_num_merge_cand = bs.getGolombU();
    }
    pslice -> slice_qp_delta = bs.getGolombS();

    if(ppps -> pps_slice_chroma_qp_offsets_present_flag)
    {
      pslice -> slice_cb_qp_offset = bs.getGolombS();
      pslice -> slice_cr_qp_offset = bs.getGolombS();
    }

    if(ppps -> deblocking_filter_override_enabled_flag)
      pslice -> deblocking_filter_override_flag = bs.getBits(1);

    if(pslice -> deblocking_filter_override_flag)
    {
      pslice -> slice_deblocking_filter_disabled_flag = bs.getBits(1);
      if(!pslice -> slice_deblocking_filter_disabled_flag)
      {
        pslice -> slice_beta_offset_div2 = bs.getGolombS();
        pslice -> slice_tc_offset_div2 = bs.getGolombS();
      }
    }
    else
    {
      pslice -> slice_deblocking_filter_disabled_flag = ppps -> pps_deblocking_filter_disabled_flag;
    }

    if(ppps -> pps_loop_filter_across_slices_enabled_flag && 
      (pslice -> slice_sao_luma_flag || pslice -> slice_sao_chroma_flag || !pslice -> slice_deblocking_filter_disabled_flag))
    {
      pslice -> slice_loop_filter_across_slices_enabled_flag = bs.getBits(1);
    }

    if(ppps -> tiles_enabled_flag || ppps -> entropy_coding_sync_enabled_flag)
    {
      pslice -> num_entry_point_offsets = bs.getGolombU();
      if(pslice -> num_entry_point_offsets > 0)
      {
        pslice -> offset_len_minus1 = bs.getGolombU();
        pslice -> entry_point_offset_minus1.resize(pslice -> num_entry_point_offsets);

        if(pslice -> offset_len_minus1 > 31)
        {
          std::stringstream ss;
          ss << "offset_len_minus1 = " 
            << (int) pslice -> offset_len_minus1 
            << ", but must be in range (0-31)";

          onWarning(ss.str(), &info, Parser::OUT_OF_RANGE);

          pslice -> m_processFailed = true;

          return;
        }
        for(std::size_t i=0; i<pslice -> num_entry_point_offsets; i++)
          pslice -> entry_point_offset_minus1[i] = bs.getBits(pslice -> offset_len_minus1 + 1);
      }
    }
  }

  if(ppps -> slice_segment_header_extension_present_flag)
  {
    pslice -> slice_segment_header_extension_length = bs.getGolombU();
    pslice -> slice_segment_header_extension_data_byte.resize(pslice -> slice_segment_header_extension_length);
    for(std::size_t i=0; i<pslice -> slice_segment_header_extension_length; i++)
      pslice -> slice_segment_header_extension_data_byte[i] = bs.getBits(8);
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
  bs.getBits(16);

  pvps -> profile_tier_level = processProfileTierLevel(pvps -> vps_max_sub_layers_minus1, bs, info);

  pvps -> vps_sub_layer_ordering_info_present_flag = bs.getBits(1);

  pvps -> vps_max_dec_pic_buffering_minus1.resize(pvps -> vps_max_sub_layers_minus1 + 1, 0);
  pvps -> vps_max_num_reorder_pics.resize(pvps -> vps_max_sub_layers_minus1 + 1, 0);
  pvps -> vps_max_latency_increase_plus1.resize(pvps -> vps_max_sub_layers_minus1 + 1, 0);


  for(std::size_t i = (pvps -> vps_sub_layer_ordering_info_present_flag ? 0 : pvps -> vps_max_sub_layers_minus1); i <= pvps -> vps_max_sub_layers_minus1; i++)
  {
    pvps -> vps_max_dec_pic_buffering_minus1[i] = bs.getGolombU();
    pvps -> vps_max_num_reorder_pics[i] = bs.getGolombU();
    pvps -> vps_max_latency_increase_plus1[i] = bs.getGolombU();
  }

  pvps -> vps_max_layer_id = bs.getBits(6);
  pvps -> vps_num_layer_sets_minus1 = bs.getGolombU();

  pvps -> layer_id_included_flag.resize(pvps -> vps_num_layer_sets_minus1+1);

  for(std::size_t i=1; i<=pvps -> vps_num_layer_sets_minus1; i++)
  {
    pvps -> layer_id_included_flag[i].resize(pvps -> vps_max_layer_id+1);
    for(std::size_t j=0; j<=pvps -> vps_max_layer_id; j++)
    {
      (pvps -> layer_id_included_flag[i])[j] = bs.getBits(1);
    }
  }

  pvps -> vps_timing_info_present_flag = bs.getBits(1);
  if(pvps -> vps_timing_info_present_flag)
  {
    pvps -> vps_num_units_in_tick = bs.getBits(32);
    pvps -> vps_time_scale = bs.getBits(32);
    pvps -> vps_poc_proportional_to_timing_flag = bs.getBits(1);

    if(pvps -> vps_poc_proportional_to_timing_flag)
    {
      pvps -> vps_num_ticks_poc_diff_one_minus1 = bs.getBits(1);
    }
    pvps -> vps_num_hrd_parameters = bs.getGolombU();

    if(pvps -> vps_num_hrd_parameters > 0)
    {
      pvps -> hrd_layer_set_idx.resize(pvps -> vps_num_hrd_parameters);
      pvps -> cprms_present_flag.resize(pvps -> vps_num_hrd_parameters);
      pvps -> cprms_present_flag[0] = 1;

      for(std::size_t i=0; i<pvps -> vps_num_hrd_parameters; i++)
      {
        pvps -> hrd_layer_set_idx[i] = bs.getGolombU();

        if(i > 0)
          pvps -> cprms_present_flag[i] = bs.getBits(1);
        pvps -> hrd_parameters.push_back(processHrdParameters(pvps -> cprms_present_flag[i], pvps -> vps_max_sub_layers_minus1, bs));
      }
    }
  }

  pvps -> vps_extension_flag = bs.getBits(1);
}


void HevcParserImpl::processSPS(std::shared_ptr<SPS> psps, BitstreamReader &bs, const Parser::Info &info)
{
  psps -> sps_video_parameter_set_id = bs.getBits(4);
  psps -> sps_max_sub_layers_minus1 = bs.getBits(3);
  psps -> sps_temporal_id_nesting_flag = bs.getBits(1);
  psps -> profile_tier_level = processProfileTierLevel(psps -> sps_max_sub_layers_minus1, bs, info);

  psps -> sps_seq_parameter_set_id = bs.getGolombU();
//  psps -> sps_seq_parameter_set_id = 0;
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
      psps -> scaling_list_data = processScalingListData(bs);
    }
  }

  psps -> amp_enabled_flag = bs.getBits(1);
  psps -> sample_adaptive_offset_enabled_flag = bs.getBits(1);
  psps -> pcm_enabled_flag = bs.getBits(1);

  if(psps -> pcm_enabled_flag)
  {
    psps -> pcm_sample_bit_depth_luma_minus1 = bs.getBits(4);
    psps -> pcm_sample_bit_depth_chroma_minus1 = bs.getBits(4);
    psps -> log2_min_pcm_luma_coding_block_size_minus3 = bs.getGolombU();
    psps -> log2_diff_max_min_pcm_luma_coding_block_size = bs.getGolombU();
    psps -> pcm_loop_filter_disabled_flag = bs.getBits(1);
  }

  psps -> num_short_term_ref_pic_sets = bs.getGolombU();

  psps -> short_term_ref_pic_set.resize(psps -> num_short_term_ref_pic_sets);
  for(std::size_t i=0; i<psps -> num_short_term_ref_pic_sets; i++)
    psps -> short_term_ref_pic_set[i] = processShortTermRefPicSet(i, psps -> num_short_term_ref_pic_sets, psps -> short_term_ref_pic_set, psps, bs, info);

  psps -> long_term_ref_pics_present_flag = bs.getBits(1);
  if(psps -> long_term_ref_pics_present_flag)
  {
    psps -> num_long_term_ref_pics_sps = bs.getGolombU();
    psps -> lt_ref_pic_poc_lsb_sps.resize(psps -> num_long_term_ref_pics_sps);
    psps -> used_by_curr_pic_lt_sps_flag.resize(psps -> num_long_term_ref_pics_sps);

    for(std::size_t i = 0; i<psps -> num_long_term_ref_pics_sps; i++)
    {
      psps -> lt_ref_pic_poc_lsb_sps[i] = bs.getBits(psps -> log2_max_pic_order_cnt_lsb_minus4 + 4);
      psps -> used_by_curr_pic_lt_sps_flag[i] = bs.getBits(1);
    }
  }

  psps -> sps_temporal_mvp_enabled_flag = bs.getBits(1);
  psps -> strong_intra_smoothing_enabled_flag = bs.getBits(1);
  psps -> vui_parameters_present_flag = bs.getBits(1);

  if(psps -> vui_parameters_present_flag)
  {
    psps -> vui_parameters = processVuiParameters(psps -> sps_max_sub_layers_minus1, bs);
  }

  psps -> sps_extension_flag = bs.getBits(1);
}



void HevcParserImpl::processAUD(std::shared_ptr<AUD> paud, BitstreamReader &bs, const Parser::Info &info)
{
  paud -> pic_type = bs.getBits(3);
}


void HevcParserImpl::processSEI(std::shared_ptr<SEI> psei, BitstreamReader &bs, const Parser::Info &info)
{
  psei -> toDefault();

  do
  {
    SeiMessage msg;
    msg.toDefault();
    msg.last_payload_type_byte = bs.getBits(8);
    while(msg.last_payload_type_byte == 0xff)
    {
      msg.num_payload_type_ff_bytes++;
      msg.last_payload_type_byte = bs.getBits(8);
    }

    msg.last_payload_size_byte = bs.getBits(8);
    std::size_t payloadSize = 0;
    while(msg.last_payload_size_byte == 0xff)
    {
      msg.num_payload_size_ff_bytes++;
      payloadSize += 255;
      msg.last_payload_size_byte = bs.getBits(8);
    }
    payloadSize += msg.last_payload_size_byte;

    if(payloadSize * 8 > bs.availableInNalU())
    {
      std::stringstream ss;
      ss << "SEI: sei message payload size more then nal unit size (payloadSize=" 
          << payloadSize
          <<")";

      onWarning(ss.str(), &info, Parser::OUT_OF_RANGE);
      break;
    }
    bs.skipBits(payloadSize * 8);

    psei -> sei_message.push_back(msg);
  }
  while(bs.available() > 8 && bs.showBits(8) != 0x80);

}


ProfileTierLevel HevcParserImpl::processProfileTierLevel(std::size_t max_sub_layers_minus1, BitstreamReader &bs, const Parser::Info &info)
{
  ProfileTierLevel ptl;

  ptl.toDefault();

  ptl.general_profile_space = bs.getBits(2);
  ptl.general_tier_flag = bs.getBits(1);
  ptl.general_profile_idc = bs.getBits(5);

  for(std::size_t i=0; i<32; i++)
    ptl.general_profile_compatibility_flag[i] = bs.getBits(1);

  ptl.general_progressive_source_flag = bs.getBits(1);
  ptl.general_interlaced_source_flag = bs.getBits(1);
  ptl.general_non_packed_constraint_flag = bs.getBits(1);
  ptl.general_frame_only_constraint_flag = bs.getBits(1);
  bs.getBits(32);
  bs.getBits(12);
  ptl.general_level_idc = bs.getBits(8);

  ptl.sub_layer_profile_present_flag.resize(max_sub_layers_minus1);
  ptl.sub_layer_level_present_flag.resize(max_sub_layers_minus1);

  for(std::size_t i=0; i<max_sub_layers_minus1; i++)
  {
    ptl.sub_layer_profile_present_flag[i] = bs.getBits(1);
    ptl.sub_layer_level_present_flag[i] = bs.getBits(1);
  }


  if(max_sub_layers_minus1 > 0)
  {
    for(std::size_t i=max_sub_layers_minus1; i<8; i++)
      bs.getBits(2);
  }

  ptl.sub_layer_profile_space.resize(max_sub_layers_minus1);
  ptl.sub_layer_tier_flag.resize(max_sub_layers_minus1);
  ptl.sub_layer_profile_idc.resize(max_sub_layers_minus1);
  ptl.sub_layer_profile_compatibility_flag.resize(max_sub_layers_minus1);
  ptl.sub_layer_progressive_source_flag.resize(max_sub_layers_minus1);
  ptl.sub_layer_interlaced_source_flag.resize(max_sub_layers_minus1);
  ptl.sub_layer_non_packed_constraint_flag.resize(max_sub_layers_minus1);
  ptl.sub_layer_frame_only_constraint_flag.resize(max_sub_layers_minus1);
  ptl.sub_layer_level_idc.resize(max_sub_layers_minus1);

  for(std::size_t i=0; i<max_sub_layers_minus1; i++)
  {
    if(ptl.sub_layer_profile_present_flag[i])
    {
      ptl.sub_layer_profile_space[i] = bs.getBits(2);
      ptl.sub_layer_tier_flag[i] = bs.getBits(1);
      ptl.sub_layer_profile_idc[i] = bs.getBits(5);
      ptl.sub_layer_profile_compatibility_flag[i].resize(32);

      for(std::size_t j=0; j<32; j++)
        ptl.sub_layer_profile_compatibility_flag[i][j] = bs.getBits(1);

      ptl.sub_layer_progressive_source_flag[i] = bs.getBits(1);
      ptl.sub_layer_interlaced_source_flag[i] = bs.getBits(1);
      ptl.sub_layer_non_packed_constraint_flag[i] = bs.getBits(1);
      ptl.sub_layer_frame_only_constraint_flag[i] = bs.getBits(1);
    }

    if(ptl.sub_layer_level_present_flag[i])
    {
      ptl.sub_layer_level_idc[i] = bs.getBits(8);
    }
    else
      ptl.sub_layer_level_idc[i] = 1;

  }

  return ptl;
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
  else
    ppps -> diff_cu_qp_delta_depth = 0;

  ppps -> pps_cb_qp_offset = bs.getGolombS();
  ppps -> pps_cr_qp_offset = bs.getGolombS();
  ppps -> pps_slice_chroma_qp_offsets_present_flag = bs.getBits(1);
  ppps -> weighted_pred_flag = bs.getBits(1);
  ppps -> weighted_bipred_flag = bs.getBits(1);
  ppps -> transquant_bypass_enabled_flag = bs.getBits(1);
  ppps -> tiles_enabled_flag = bs.getBits(1);
  ppps -> entropy_coding_sync_enabled_flag = bs.getBits(1);

  if(ppps -> tiles_enabled_flag)
  {
    ppps -> num_tile_columns_minus1 = bs.getGolombU();
    ppps -> num_tile_rows_minus1 = bs.getGolombU();
    ppps -> uniform_spacing_flag = bs.getBits(1);

    if(!ppps -> uniform_spacing_flag)
    {
      ppps -> column_width_minus1.resize(ppps -> num_tile_columns_minus1);
      for(std::size_t i=0; i<ppps -> num_tile_columns_minus1; i++)
        ppps -> column_width_minus1[i] = bs.getGolombU();

      ppps -> row_height_minus1.resize(ppps -> num_tile_rows_minus1);
      for(std::size_t i=0; i<ppps -> num_tile_rows_minus1; i++)
        ppps -> row_height_minus1[i] = bs.getGolombU();
    }
    ppps -> loop_filter_across_tiles_enabled_flag = bs.getBits(1);
  }
  else
  {
    ppps -> num_tile_columns_minus1 = 0;
    ppps -> num_tile_rows_minus1 = 0;
    ppps -> uniform_spacing_flag = 1;
    ppps -> loop_filter_across_tiles_enabled_flag = 1;
  }

  ppps -> pps_loop_filter_across_slices_enabled_flag = bs.getBits(1);
  ppps -> deblocking_filter_control_present_flag = bs.getBits(1);

  if(ppps -> deblocking_filter_control_present_flag)
  {
    ppps -> deblocking_filter_override_enabled_flag = bs.getBits(1);
    ppps -> pps_deblocking_filter_disabled_flag = bs.getBits(1);

    if(!ppps -> pps_deblocking_filter_disabled_flag)
    {
      ppps -> pps_beta_offset_div2 = bs.getGolombS();
      ppps -> pps_tc_offset_div2 = bs.getGolombS();
    }
    else
    {
      ppps -> pps_beta_offset_div2 = 0;
      ppps -> pps_tc_offset_div2 = 0;
    }
  }
  else
  {
    ppps -> deblocking_filter_override_enabled_flag = 0;
    ppps -> pps_deblocking_filter_disabled_flag = 0;
  }

  ppps -> pps_scaling_list_data_present_flag = bs.getBits(1);
  if(ppps -> pps_scaling_list_data_present_flag)
  {
    ppps -> scaling_list_data = processScalingListData(bs);
  }

  ppps -> lists_modification_present_flag = bs.getBits(1);
  ppps -> log2_parallel_merge_level_minus2 = bs.getGolombU();
  ppps -> slice_segment_header_extension_present_flag = bs.getBits(1);
  ppps -> pps_extension_flag = bs.getBits(1);
}



HrdParameters HevcParserImpl::processHrdParameters(uint8_t commonInfPresentFlag, std::size_t maxNumSubLayersMinus1, BitstreamReader &bs)
{
  HrdParameters hrd;

  hrd.toDefault();

  hrd.nal_hrd_parameters_present_flag = 0;
  hrd.vcl_hrd_parameters_present_flag = 0;
  hrd.sub_pic_hrd_params_present_flag = 0;
  hrd.sub_pic_cpb_params_in_pic_timing_sei_flag = 0;
  if(commonInfPresentFlag)
  {
    hrd.nal_hrd_parameters_present_flag = bs.getBits(1);
    hrd.vcl_hrd_parameters_present_flag = bs.getBits(1);

    if(hrd.nal_hrd_parameters_present_flag || hrd.vcl_hrd_parameters_present_flag)
    {
      hrd.sub_pic_hrd_params_present_flag = bs.getBits(1);
      if(hrd.sub_pic_hrd_params_present_flag)
      {
        hrd.tick_divisor_minus2 = bs.getBits(8);
        hrd.du_cpb_removal_delay_increment_length_minus1 = bs.getBits(5);
        hrd.sub_pic_cpb_params_in_pic_timing_sei_flag = bs.getBits(1);
        hrd.dpb_output_delay_du_length_minus1 = bs.getBits(5);
      }
      hrd.bit_rate_scale = bs.getBits(4);
      hrd.cpb_size_scale = bs.getBits(4);

      if(hrd.sub_pic_hrd_params_present_flag)
        hrd.cpb_size_du_scale = bs.getBits(4);

      hrd.initial_cpb_removal_delay_length_minus1 = bs.getBits(5);
      hrd.au_cpb_removal_delay_length_minus1 = bs.getBits(5);
      hrd.dpb_output_delay_length_minus1 = bs.getBits(5);
    }
  }

  hrd.fixed_pic_rate_general_flag.resize(maxNumSubLayersMinus1 + 1);
  hrd.fixed_pic_rate_within_cvs_flag.resize(maxNumSubLayersMinus1 + 1);
  hrd.elemental_duration_in_tc_minus1.resize(maxNumSubLayersMinus1 + 1);
  hrd.low_delay_hrd_flag.resize(maxNumSubLayersMinus1 + 1, 0);
  hrd.cpb_cnt_minus1.resize(maxNumSubLayersMinus1 + 1, 0);

  if(hrd.nal_hrd_parameters_present_flag)
    hrd.nal_sub_layer_hrd_parameters.resize(maxNumSubLayersMinus1 + 1);
  if(hrd.vcl_hrd_parameters_present_flag)
    hrd.vcl_sub_layer_hrd_parameters.resize(maxNumSubLayersMinus1 + 1);

  for(std::size_t i = 0; i <= maxNumSubLayersMinus1; i++ )
  {
    hrd.fixed_pic_rate_general_flag[i] = bs.getBits(1);

    if(hrd.fixed_pic_rate_general_flag[i])
      hrd.fixed_pic_rate_within_cvs_flag[i] = 1;

    if(!hrd.fixed_pic_rate_general_flag[i])
       hrd.fixed_pic_rate_within_cvs_flag[i] = bs.getBits(1);

    if(hrd.fixed_pic_rate_within_cvs_flag[i])
      hrd.elemental_duration_in_tc_minus1[i] = bs.getGolombU();
    else
      hrd.low_delay_hrd_flag[i] = bs.getBits(1);

    if(!hrd.low_delay_hrd_flag[i])
      hrd.cpb_cnt_minus1[i] = bs.getGolombU();

    if(hrd.nal_hrd_parameters_present_flag)
      hrd.nal_sub_layer_hrd_parameters[i] = processSubLayerHrdParameters(hrd.sub_pic_hrd_params_present_flag, hrd.cpb_cnt_minus1[i], bs);
    if(hrd.vcl_hrd_parameters_present_flag)
      hrd.vcl_sub_layer_hrd_parameters[i] = processSubLayerHrdParameters(hrd.sub_pic_hrd_params_present_flag, hrd.cpb_cnt_minus1[i], bs);
  }

  return hrd;
}


SubLayerHrdParameters HevcParserImpl::processSubLayerHrdParameters(uint8_t sub_pic_hrd_params_present_flag, std::size_t CpbCnt, BitstreamReader &bs)
{
  SubLayerHrdParameters slhrd;

  slhrd.toDefault();
  slhrd.bit_rate_value_minus1.resize(CpbCnt + 1);
  slhrd.cpb_size_value_minus1.resize(CpbCnt + 1);
  slhrd.cpb_size_du_value_minus1.resize(CpbCnt + 1);
  slhrd.bit_rate_du_value_minus1.resize(CpbCnt + 1);
  slhrd.cbr_flag.resize(CpbCnt + 1);

  for(std::size_t i=0; i<=CpbCnt; i++)
  {
    slhrd.bit_rate_value_minus1[i] = bs.getGolombU();
    slhrd.cpb_size_value_minus1[i] = bs.getGolombU();

    if(sub_pic_hrd_params_present_flag)
    {
      slhrd.cpb_size_du_value_minus1[i] = bs.getGolombU();
      slhrd.bit_rate_du_value_minus1[i] = bs.getGolombU();
    }

    slhrd.cbr_flag[i] = bs.getBits(1);
  }

  return slhrd;
}



ShortTermRefPicSet HevcParserImpl::processShortTermRefPicSet(std::size_t stRpsIdx, std::size_t num_short_term_ref_pic_sets, const std::vector<ShortTermRefPicSet> &refPicSets, std::shared_ptr<SPS> psps, BitstreamReader &bs, const Parser::Info &info)
{
  ShortTermRefPicSet rpset;

  rpset.toDefault();

  rpset.inter_ref_pic_set_prediction_flag = 0;
  rpset.delta_idx_minus1 = 0;
  if(stRpsIdx)
  {
    rpset.inter_ref_pic_set_prediction_flag = bs.getBits(1);
  }

  if(rpset.inter_ref_pic_set_prediction_flag)
  {
    if(stRpsIdx == num_short_term_ref_pic_sets)
      rpset.delta_idx_minus1 = bs.getGolombU();

    rpset.delta_rps_sign = bs.getBits(1);
    rpset.abs_delta_rps_minus1 = bs.getGolombU();

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

    rpset.used_by_curr_pic_flag.resize(NumDeltaPocs + 1);
    rpset.use_delta_flag.resize(NumDeltaPocs + 1, 1);

    for(std::size_t i=0; i<=NumDeltaPocs; i++ )
    {
      rpset.used_by_curr_pic_flag[i] = bs.getBits(1);
      if(!rpset.used_by_curr_pic_flag[i])
        rpset.use_delta_flag[i] = bs.getBits(1);
    }
  }
  else
  {
    rpset.num_negative_pics = bs.getGolombU();
    rpset.num_positive_pics = bs.getGolombU();

    if(rpset.num_negative_pics > psps -> sps_max_dec_pic_buffering_minus1[psps -> sps_max_sub_layers_minus1])
    {
      onWarning("ShortTermRefPicSet: num_negative_pics > sps_max_dec_pic_buffering_minus1", &info, Parser::OUT_OF_RANGE);
      return rpset;
    }

    if(rpset.num_positive_pics > psps -> sps_max_dec_pic_buffering_minus1[psps -> sps_max_sub_layers_minus1])
    {
      onWarning("ShortTermRefPicSet: num_positive_pics > sps_max_dec_pic_buffering_minus1", &info, Parser::OUT_OF_RANGE);
      return rpset;      
    }

    rpset.delta_poc_s0_minus1.resize(rpset.num_negative_pics);
    rpset.used_by_curr_pic_s0_flag.resize(rpset.num_negative_pics);

    for(std::size_t i=0; i<rpset.num_negative_pics; i++)
    {
      rpset.delta_poc_s0_minus1[i] = bs.getGolombU();
      rpset.used_by_curr_pic_s0_flag[i] = bs.getBits(1);
    }

    rpset.delta_poc_s1_minus1.resize(rpset.num_positive_pics);
    rpset.used_by_curr_pic_s1_flag.resize(rpset.num_positive_pics);
    for(std::size_t i=0; i<rpset.num_positive_pics; i++)
    {
      rpset.delta_poc_s1_minus1[i] = bs.getGolombU();
      rpset.used_by_curr_pic_s1_flag[i] = bs.getBits(1);
    }

  }

  return rpset;
}


VuiParameters HevcParserImpl::processVuiParameters(std::size_t sps_max_sub_layers_minus1, BitstreamReader &bs)
{
  VuiParameters vui;

  vui.toDefault();

  vui.aspect_ratio_idc = 0;
  vui.sar_width = 0;
  vui.sar_height = 0;


  vui.aspect_ratio_info_present_flag = bs.getBits(1);

  if(vui.aspect_ratio_info_present_flag)
  {
    vui.aspect_ratio_idc = bs.getBits(8);

    if(vui.aspect_ratio_idc == 255) //EXTENDED_SAR
    {
      vui.sar_width = bs.getBits(16);
      vui.sar_height = bs.getBits(16);
    }
  }


  vui.overscan_info_present_flag = bs.getBits(1);
  if(vui.overscan_info_present_flag)
    vui.overscan_appropriate_flag = bs.getBits(1);

  vui.video_format = 5;
  vui.video_full_range_flag = 0;
  vui.colour_primaries = 2;
  vui.transfer_characteristics = 2;
  vui.matrix_coeffs = 2;

  vui.video_signal_type_present_flag = bs.getBits(1);

  if(vui.video_signal_type_present_flag)
  {
    vui.video_format = bs.getBits(3);
    vui.video_full_range_flag = bs.getBits(1);
    vui.colour_description_present_flag = bs.getBits(1);

    if(vui.colour_description_present_flag)
    {
      vui.colour_primaries = bs.getBits(8);
      vui.transfer_characteristics = bs.getBits(8);
      vui.matrix_coeffs = bs.getBits(8);
    }

  }

  vui.chroma_sample_loc_type_top_field = 0;
  vui.chroma_sample_loc_type_bottom_field = 0;

  vui.chroma_loc_info_present_flag = bs.getBits(1);
  if(vui.chroma_loc_info_present_flag)
  {
    vui.chroma_sample_loc_type_top_field = bs.getGolombU();
    vui.chroma_sample_loc_type_bottom_field = bs.getGolombU();
  }


  vui.neutral_chroma_indication_flag = bs.getBits(1);
  vui.field_seq_flag = bs.getBits(1);
  vui.frame_field_info_present_flag = bs.getBits(1);
  vui.default_display_window_flag = bs.getBits(1);

  vui.def_disp_win_left_offset = 0;
  vui.def_disp_win_right_offset = 0;
  vui.def_disp_win_right_offset = 0;
  vui.def_disp_win_bottom_offset = 0;

  if(vui.default_display_window_flag)
  {
    vui.def_disp_win_left_offset = bs.getGolombU();
    vui.def_disp_win_right_offset = bs.getGolombU();
    vui.def_disp_win_top_offset = bs.getGolombU();
    vui.def_disp_win_bottom_offset = bs.getGolombU();
  }

  vui.vui_timing_info_present_flag = bs.getBits(1);

  if(vui.vui_timing_info_present_flag)
  {
    vui.vui_num_units_in_tick = bs.getBits(32);
    vui.vui_time_scale = bs.getBits(32);
    vui.vui_poc_proportional_to_timing_flag = bs.getBits(1);

    if(vui.vui_poc_proportional_to_timing_flag)
      vui.vui_num_ticks_poc_diff_one_minus1 = bs.getGolombU();

    vui.vui_hrd_parameters_present_flag = bs.getBits(1);

    if(vui.vui_hrd_parameters_present_flag)
      vui.hrd_parameters = processHrdParameters(1, sps_max_sub_layers_minus1, bs);
  }

  vui.bitstream_restriction_flag = bs.getBits(1);

  if(vui.bitstream_restriction_flag)
  {
    vui.tiles_fixed_structure_flag = bs.getBits(1);
    vui.motion_vectors_over_pic_boundaries_flag = bs.getBits(1);
    vui.restricted_ref_pic_lists_flag = bs.getBits(1);

    vui.min_spatial_segmentation_idc = bs.getGolombU();
    vui.max_bytes_per_pic_denom = bs.getGolombU();
    vui.max_bits_per_min_cu_denom = bs.getGolombU();
    vui.log2_max_mv_length_horizontal = bs.getGolombU();
    vui.log2_max_mv_length_vertical = bs.getGolombU();
  }

  return vui;
}


ScalingListData HevcParserImpl::processScalingListData(BitstreamReader &bs)
{
  ScalingListData sc;

  sc.scaling_list_pred_mode_flag.resize(4);
  sc.scaling_list_pred_matrix_id_delta.resize(4);
  sc.scaling_list_dc_coef_minus8.resize(2);
  sc.scaling_list_delta_coef.resize(4);

  for(std::size_t sizeId = 0; sizeId < 4; sizeId++)
  {
    if(sizeId == 3)
    {
      sc.scaling_list_pred_mode_flag[sizeId].resize(2);
      sc.scaling_list_pred_matrix_id_delta[sizeId].resize(2);
      sc.scaling_list_dc_coef_minus8[sizeId-2].resize(2);
      sc.scaling_list_delta_coef[sizeId].resize(2);
    }
    else
    {
      sc.scaling_list_pred_mode_flag[sizeId].resize(6);
      sc.scaling_list_pred_matrix_id_delta[sizeId].resize(6);
      sc.scaling_list_delta_coef[sizeId].resize(6);
      if(sizeId >= 2)
        sc.scaling_list_dc_coef_minus8[sizeId-2].resize(6);
    }

    for(std::size_t matrixId = 0; matrixId<((sizeId == 3)?2:6); matrixId++)
    {
      sc.scaling_list_pred_mode_flag[sizeId][matrixId] = bs.getBits(1);
      if(!sc.scaling_list_pred_mode_flag[sizeId][matrixId])
        sc.scaling_list_pred_matrix_id_delta[sizeId][matrixId] = bs.getGolombU();
      else
      {
        std::size_t nextCoef = 8;
        std::size_t coefNum = std::min(64, (1 << (4 + (sizeId << 1))));
        if(sizeId > 1)
          sc.scaling_list_dc_coef_minus8[sizeId-2][matrixId] = bs.getGolombS();

        sc.scaling_list_delta_coef[sizeId][matrixId].resize(coefNum);
        for(std::size_t i = 0; i < coefNum; i++)
          sc.scaling_list_delta_coef[sizeId][matrixId][i] = bs.getGolombS();
      }
    }
  }

  return sc;
}


RefPicListModification HevcParserImpl::processRefPicListModification(BitstreamReader &bs, std::shared_ptr<Slice> pslice)
{
  RefPicListModification res;
  res.toDefault();


  if(!m_ppsMap[pslice -> slice_pic_parameter_set_id])
  {
    assert(0);
    return res;
  }

  int32_t spsId = m_ppsMap[pslice -> slice_pic_parameter_set_id] -> pps_seq_parameter_set_id;

  if(!m_spsMap[spsId])
  {
    assert(0);
    return res;
  }

  std::size_t numPocTotalCurr = calcNumPocTotalCurr(pslice, m_spsMap[spsId]);
  int32_t listSize = HEVC::log2(numPocTotalCurr);

  if((1 << listSize) < numPocTotalCurr)
    listSize++;


  res.ref_pic_list_modification_flag_l0 = bs.getBits(1);

  if(res.ref_pic_list_modification_flag_l0)
  {
    res.list_entry_l0.resize(pslice -> num_ref_idx_l0_active_minus1);

    for(std::size_t i=0; i<pslice -> num_ref_idx_l0_active_minus1; i++)
      res.list_entry_l0[i] = bs.getBits(listSize);
  }

  res.ref_pic_list_modification_flag_l1 = bs.getBits(1);

  if(res.ref_pic_list_modification_flag_l1)
  {
    res.list_entry_l1.resize(pslice -> num_ref_idx_l1_active_minus1);

    for(std::size_t i=0; i<pslice -> num_ref_idx_l1_active_minus1; i++)
      res.list_entry_l1[i] = bs.getBits(listSize);
  }

  return res;
}



PredWeightTable HevcParserImpl::processPredWeightTable(BitstreamReader &bs, std::shared_ptr<Slice> pslice)
{
  PredWeightTable pwt;
  pwt.toDefault();

  std::shared_ptr<PPS> ppps = m_ppsMap[pslice -> slice_pic_parameter_set_id];
  if(!ppps)
    return pwt;

  std::shared_ptr<SPS> psps = m_spsMap[ppps -> pps_seq_parameter_set_id];

  if(!psps)
    return pwt;

  pwt.luma_log2_weight_denom = bs.getGolombU();
  if(psps -> chroma_format_idc != 0)
    pwt.delta_chroma_log2_weight_denom = bs.getGolombS();

  pwt.luma_weight_l0_flag.resize(pslice -> num_ref_idx_l0_active_minus1);

  for(std::size_t i=0; i<pslice -> num_ref_idx_l0_active_minus1; i++)
    pwt.luma_weight_l0_flag[i] = bs.getBits(1);

  pwt.chroma_weight_l0_flag.resize(pslice -> num_ref_idx_l0_active_minus1, 0);

  if(psps -> chroma_format_idc != 0)
  {
    for(std::size_t i=0; i<pslice -> num_ref_idx_l0_active_minus1; i++)
      pwt.chroma_weight_l0_flag[i] = bs.getBits(1);
  }

  pwt.delta_luma_weight_l0.resize(pslice -> num_ref_idx_l0_active_minus1);
  pwt.luma_offset_l0.resize(pslice -> num_ref_idx_l0_active_minus1);

  pwt.delta_chroma_weight_l0.resize(pslice -> num_ref_idx_l0_active_minus1);
  pwt.delta_chroma_offset_l0.resize(pslice -> num_ref_idx_l0_active_minus1);

  for(std::size_t i=0; i<pslice -> num_ref_idx_l0_active_minus1; i++)
  {
    if(pwt.luma_weight_l0_flag[i])
    {
      pwt.delta_luma_weight_l0[i] = bs.getGolombS();
      pwt.luma_offset_l0[i] = bs.getGolombS();
    }
    if(pwt.chroma_weight_l0_flag[i])
    {
      for(std::size_t j=0; j<3; j++)
      {
        pwt.delta_chroma_weight_l0[i][j] = bs.getGolombS();
        pwt.delta_chroma_offset_l0[i][j] = bs.getGolombS();
      }
    }
  }

  if(pslice -> slice_type == SLICE_B)
  {
    pwt.luma_weight_l1_flag.resize(pslice -> num_ref_idx_l1_active_minus1 + 1);
    
    for(std::size_t i=0; i<=pslice -> num_ref_idx_l1_active_minus1; i++)
      pwt.luma_weight_l1_flag[i] = bs.getBits(1);

    pwt.chroma_weight_l1_flag.resize(pslice -> num_ref_idx_l1_active_minus1 + 1, 0);

    if(psps -> chroma_format_idc != 0)
    {
      for(std::size_t i=0; i<=pslice -> num_ref_idx_l1_active_minus1; i++)
        pwt.chroma_weight_l1_flag[i] = bs.getBits(1);
    }

    pwt.delta_luma_weight_l1.resize(pslice -> num_ref_idx_l1_active_minus1 + 1);
    pwt.luma_offset_l1.resize(pslice -> num_ref_idx_l1_active_minus1 + 1);
    pwt.delta_chroma_weight_l1.resize(pslice -> num_ref_idx_l1_active_minus1 + 1);
    pwt.delta_chroma_offset_l1.resize(pslice -> num_ref_idx_l1_active_minus1 + 1);

    for(std::size_t i=0; i<=pslice -> num_ref_idx_l1_active_minus1; i++)
    {
      if(pwt.luma_weight_l1_flag[i])
      {
        pwt.delta_luma_weight_l1[i] = bs.getGolombS();
        pwt.luma_offset_l1[i] = bs.getGolombS();
      }
      if(pwt.chroma_weight_l1_flag[i])
      {
        for(std::size_t j=0; j<3; j++)
        {
          pwt.delta_chroma_weight_l1[i][j] = bs.getGolombS();
          pwt.delta_chroma_offset_l1[i][j] = bs.getGolombS();
        }
      }
    }
  } 
}
