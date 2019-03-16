#include "ProfileConformanceAnalyzer.h"

#include <limits>
#include <string>
#include <sstream>

#include <QDebug>

ProfileConformanceAnalyzer::ProfileConformanceAnalyzer():
  m_pconsumer(NULL)
  ,m_profile(std::numeric_limits<uint32_t>::max())
{

}


std::string ProfileConformanceAnalyzer::profileName(uint32_t profile)
{
  std::string name;
  switch(profile)
  {
    case 1:
      name = "Main";
      break;
    case 2:
      name = "Main 10";
      break;
    case 3:
      name = "Main Still Picture";
      break;

    case std::numeric_limits<uint32_t>::max():
      name = "NOT PRESENT";
      break;

    default:
      name = "UNKNOWN";
      break;
  }

  return name;
}


void ProfileConformanceAnalyzer::sendWarning(const std::string &warning, const HEVC::Parser::Info *pInfo)
{
  if(m_pconsumer)
    m_pconsumer -> onWarning(warning, pInfo, HEVC::Parser::PROFILE_CONFORMANCE);
}


uint32_t ProfileConformanceAnalyzer::ctbLog2SizeY(std::shared_ptr<HEVC::SPS> pSPS)
{
  int32_t CtbLog2SizeY = pSPS -> log2_min_luma_coding_block_size_minus3 + 3 + pSPS -> log2_diff_max_min_luma_coding_block_size;
  return CtbLog2SizeY;
}


std::vector<uint32_t> ProfileConformanceAnalyzer::columnWidthInLumaSamples(std::shared_ptr<HEVC::PPS> pPPS)
{
  std::shared_ptr<HEVC::SPS> pSPS = m_spsMap[pPPS -> pps_seq_parameter_set_id];

  if(!pSPS)
    return std::vector<uint32_t>();

  std::vector<uint32_t> colWidth(pPPS -> num_tile_columns_minus1 + 1);

  int32_t CtbLog2SizeY = pSPS -> log2_min_luma_coding_block_size_minus3 + 3 + pSPS -> log2_diff_max_min_luma_coding_block_size;
  uint32_t CtbSizeY = 1 << CtbLog2SizeY;
  uint32_t PicWidthInCtbsY = pSPS -> pic_width_in_luma_samples / CtbSizeY;
  if(pSPS -> pic_width_in_luma_samples % CtbSizeY)
    PicWidthInCtbsY++;


  if(pPPS -> uniform_spacing_flag)
  {
    for(std::size_t i=0; i <= pPPS -> num_tile_columns_minus1; i++)
      colWidth[i] = ((i + 1) * PicWidthInCtbsY) / (pPPS -> num_tile_columns_minus1 + 1) -
        (i * PicWidthInCtbsY) / (pPPS -> num_tile_columns_minus1 + 1);
  }
  else
  {
    colWidth[pPPS -> num_tile_columns_minus1] = PicWidthInCtbsY;
    for(std::size_t i=0; i < pPPS -> num_tile_columns_minus1; i++)
    {
      colWidth[i] = pPPS -> column_width_minus1[i] + 1;
      colWidth[pPPS -> num_tile_columns_minus1] -= colWidth[i];
    }
  }

  for(std::size_t i=0; i <= pPPS -> num_tile_columns_minus1; i++)
    colWidth[i] = colWidth[i] << CtbLog2SizeY;

  return colWidth;
}

std::vector<uint32_t> ProfileConformanceAnalyzer::rowHeightInLumaSamples(std::shared_ptr<HEVC::PPS> pPPS)
{
  std::shared_ptr<HEVC::SPS> pSPS = m_spsMap[pPPS -> pps_seq_parameter_set_id];

  if(!pSPS)
    return std::vector<uint32_t>();

  std::vector<uint32_t> rowHeight(pPPS -> num_tile_rows_minus1 + 1);

  int32_t CtbLog2SizeY = pSPS -> log2_min_luma_coding_block_size_minus3 + 3 + pSPS -> log2_diff_max_min_luma_coding_block_size;
  uint32_t CtbSizeY = 1 << CtbLog2SizeY;

  uint32_t PicHeightInCtbsY = pSPS -> pic_height_in_luma_samples / CtbSizeY;
  if(pSPS -> pic_height_in_luma_samples % CtbSizeY)
    PicHeightInCtbsY++;


  if(pPPS -> uniform_spacing_flag)
  {
    for(std::size_t i=0; i <= pPPS -> num_tile_rows_minus1; i++)
      rowHeight[i] = ((i + 1) * PicHeightInCtbsY) / (pPPS -> num_tile_rows_minus1 + 1) -
        (i * PicHeightInCtbsY) / (pPPS -> num_tile_rows_minus1 + 1);
  }
  else
  {
    rowHeight[pPPS -> num_tile_rows_minus1] = PicHeightInCtbsY;

    for(std::size_t i=0; i < pPPS -> num_tile_rows_minus1; i++)
    {
      rowHeight[i] = pPPS -> row_height_minus1[i] + 1;
      rowHeight[pPPS -> num_tile_rows_minus1] -= rowHeight[i];
    }
  }

  for(std::size_t i=0; i <= pPPS -> num_tile_columns_minus1; i++)
    rowHeight[i] = rowHeight[i] << CtbLog2SizeY;

  return rowHeight;
}


void ProfileConformanceAnalyzer::onNALUnit(std::shared_ptr<HEVC::NALUnit> pNALUnit, const HEVC::Parser::Info *pInfo)
{
  if(pNALUnit -> m_processFailed)
    return;

  switch(pNALUnit -> m_nalHeader.type)
  {
    case HEVC::NAL_VPS:
    {
      std::shared_ptr<HEVC::VPS> pVPS = std::dynamic_pointer_cast<HEVC::VPS>(pNALUnit);

      if(m_profile != std::numeric_limits<uint32_t>::max() && m_profile != pVPS -> profile_tier_level.general_profile_idc)
      {
        std::stringstream ss;
        ss <<  "Profile was changed from "
           << m_profile << " (" << profileName(m_profile) << ") "
           << "to "
           << pVPS -> profile_tier_level.general_profile_idc << " (" << profileName(pVPS -> profile_tier_level.general_profile_idc) <<  ") ";

        sendWarning(ss.str(), pInfo);
      }

      m_profile = pVPS -> profile_tier_level.general_profile_idc;

      m_vpsMap[pVPS -> vps_video_parameter_set_id] = pVPS;

      break;
    }

    case HEVC::NAL_SPS:
    {
      std::shared_ptr<HEVC::SPS> pSPS = std::dynamic_pointer_cast<HEVC::SPS>(pNALUnit);

      if(m_profile != std::numeric_limits<uint32_t>::max() && m_profile != pSPS -> profile_tier_level.general_profile_idc)
      {
        std::stringstream ss;
        ss << "Profile was changed from "
          << m_profile << " (" << profileName(m_profile) << ") "
          << "to "
          << pSPS -> profile_tier_level.general_profile_idc << " (" << profileName(pSPS -> profile_tier_level.general_profile_idc) << ") ";

        sendWarning(ss.str(), pInfo);
      }

      m_profile = pSPS -> profile_tier_level.general_profile_idc;

      m_spsMap[pSPS -> sps_seq_parameter_set_id] = pSPS;

      if(m_profile == 1)
        checkMainSPS(pSPS, pInfo);
      else if(m_profile == 2)
        checkMain10SPS(pSPS, pInfo);
      else if(m_profile == 3)
        checkMainStillPictureSPS(pSPS, pInfo);

      break;
    }

    case HEVC::NAL_PPS:
    {
      std::shared_ptr<HEVC::PPS> pPPS = std::dynamic_pointer_cast<HEVC::PPS>(pNALUnit);

      m_ppsMap[pPPS -> pps_pic_parameter_set_id] = pPPS;

      if(m_profile == 1)
        checkMainPPS(pPPS, pInfo);
      else if(m_profile == 2)
        checkMain10PPS(pPPS, pInfo);
      else if(m_profile == 3)
        checkMainStillPicturePPS(pPPS, pInfo);

      break;
    }

    default:
      break;
  }
}


void ProfileConformanceAnalyzer::checkMainSPS(std::shared_ptr<HEVC::SPS> pSPS, const HEVC::Parser::Info *pInfo)
{
  if(pSPS -> chroma_format_idc != 1)
    sendWarning("Main profile: SPSs shall have chroma_format_idc equal to 1 only", pInfo);

  if(pSPS -> bit_depth_luma_minus8)
    sendWarning("Main profile: SPSs shall have bit_depth_luma_minus8 equal to 0 only", pInfo);

  if(pSPS -> bit_depth_chroma_minus8)
    sendWarning("Main profile: SPSs shall have bit_depth_chroma_minus8 equal to 0 only", pInfo);

  if(ctbLog2SizeY(pSPS) < 4 || ctbLog2SizeY(pSPS) > 6)
    sendWarning("Main profile: CtbLog2SizeY shall be in the range of 4 to 6, inclusive.", pInfo);
}

void ProfileConformanceAnalyzer::checkMain10SPS(std::shared_ptr<HEVC::SPS> pSPS, const HEVC::Parser::Info *pInfo)
{
  if(pSPS -> chroma_format_idc != 1)
    sendWarning("Main10 profile: SPSs shall have chroma_format_idc equal to 1 only", pInfo);

  if(pSPS -> bit_depth_luma_minus8 > 2)
    sendWarning("Main10 profile: SPSs shall have bit_depth_luma_minus8 in the range of 0 to 2, inclusive", pInfo);

  if(pSPS -> bit_depth_chroma_minus8 > 2)
    sendWarning("Main10 profile: SPSs shall have bit_depth_chroma_minus8 in the range of 0 to 2, inclusive", pInfo);

  if(ctbLog2SizeY(pSPS) < 4 || ctbLog2SizeY(pSPS) > 6)
    sendWarning("Main10 profile: CtbLog2SizeY shall be in the range of 4 to 6, inclusive.", pInfo);

}


void ProfileConformanceAnalyzer::checkMainStillPictureSPS(std::shared_ptr<HEVC::SPS> pSPS, const HEVC::Parser::Info *pInfo)
{
  if(pSPS -> chroma_format_idc != 1)
    sendWarning("Main Still Picture profile: SPSs shall have chroma_format_idc equal to 1 only", pInfo);

  if(pSPS -> bit_depth_luma_minus8)
    sendWarning("Main Still Picture profile: SPSs shall have bit_depth_luma_minus8 equal to 0 only", pInfo);

  if(pSPS -> bit_depth_chroma_minus8)
    sendWarning("Main Still Picture profile: SPSs shall have bit_depth_chroma_minus8 equal to 0 only", pInfo);

  if(pSPS -> sps_max_dec_pic_buffering_minus1[pSPS -> sps_max_sub_layers_minus1])
    sendWarning("Main Still Picture profile: SPSs shall have sps_max_dec_pic_buffering_minus1[sps_max_sub_layers_minus1] equal to 0 only", pInfo);

  if(ctbLog2SizeY(pSPS) < 4 || ctbLog2SizeY(pSPS) > 6)
    sendWarning("Main10 Still Picture profile: CtbLog2SizeY shall be in the range of 4 to 6, inclusive.", pInfo);
}

void ProfileConformanceAnalyzer::checkMainPPS(std::shared_ptr<HEVC::PPS> pPPS, const HEVC::Parser::Info *pInfo)
{
  std::shared_ptr<HEVC::SPS> pSPS = m_spsMap[pPPS -> pps_seq_parameter_set_id];
  if(!pSPS)
    return;

  if(pPPS -> tiles_enabled_flag)
  {
    if(pPPS -> entropy_coding_sync_enabled_flag)
      sendWarning("Main profile: When a PPS has tiles_enabled_flag is equal to 1, it shall have entropy_coding_sync_enabled_flag equal to 0.", pInfo);

    std::vector<uint32_t> ColumnWidthInLumaSamples = columnWidthInLumaSamples(pPPS);
    std::vector<uint32_t> RowHeightInLumaSamples = rowHeightInLumaSamples(pPPS);

    bool bWarning = false;
    for(std::size_t i=0; i <= pPPS -> num_tile_columns_minus1; i++)
    {
      if(ColumnWidthInLumaSamples[i] < 256 || RowHeightInLumaSamples[i] <= 64)
        bWarning = true;
    }

    if(bWarning)
    {
      sendWarning("Main profile: When a PPS has tiles_enabled_flag is equal to 1, ColumnWidthInLumaSamples[ i ] shall be greater than or equal to "
        "256 for all values of i in the range of 0 to num_tile_columns_minus1, inclusive, and "
        "RowHeightInLumaSamples[ j ] shall be greater than or equal to 64 for all values of j in the range of 0 to "
        "num_tile_rows_minus1, inclusive.", pInfo);
    }
  }
}

void ProfileConformanceAnalyzer::checkMain10PPS(std::shared_ptr<HEVC::PPS> pPPS, const HEVC::Parser::Info *pInfo)
{
  std::shared_ptr<HEVC::SPS> pSPS = m_spsMap[pPPS -> pps_seq_parameter_set_id];

  if(!pSPS)
    return;

  if(pPPS -> tiles_enabled_flag)
  {
    if(pPPS -> entropy_coding_sync_enabled_flag)
      sendWarning("Main10 profile: When a PPS has tiles_enabled_flag is equal to 1, it shall have entropy_coding_sync_enabled_flag equal to 0.", pInfo);

    std::vector<uint32_t> ColumnWidthInLumaSamples = columnWidthInLumaSamples(pPPS);
    std::vector<uint32_t> RowHeightInLumaSamples = rowHeightInLumaSamples(pPPS);

    bool bWarning = false;
    for(std::size_t i=0; i <= pPPS -> num_tile_columns_minus1; i++)
    {
      if(ColumnWidthInLumaSamples[i] < 256 || RowHeightInLumaSamples[i] <= 64)
        bWarning = true;
    }

    if(bWarning)
    {
      sendWarning("Main10 profile: When a PPS has tiles_enabled_flag is equal to 1, ColumnWidthInLumaSamples[ i ] shall be greater than or equal to "
        "256 for all values of i in the range of 0 to num_tile_columns_minus1, inclusive, and "
        "RowHeightInLumaSamples[ j ] shall be greater than or equal to 64 for all values of j in the range of 0 to "
        "num_tile_rows_minus1, inclusive.", pInfo);
    }
  }
}

void ProfileConformanceAnalyzer::checkMainStillPicturePPS(std::shared_ptr<HEVC::PPS> pPPS, const HEVC::Parser::Info *pInfo)
{
  std::shared_ptr<HEVC::SPS> pSPS = m_spsMap[pPPS -> pps_seq_parameter_set_id];

  if(!pSPS)
    return;

  if(pPPS -> tiles_enabled_flag)
  {
    if(pPPS -> entropy_coding_sync_enabled_flag)
      sendWarning("Main Still Picture profile: When a PPS has tiles_enabled_flag is equal to 1, it shall have entropy_coding_sync_enabled_flag equal to 0.", pInfo);

    std::vector<uint32_t> ColumnWidthInLumaSamples = columnWidthInLumaSamples(pPPS);
    std::vector<uint32_t> RowHeightInLumaSamples = rowHeightInLumaSamples(pPPS);

    bool bWarning = false;
    for(std::size_t i=0; i <= pPPS -> num_tile_columns_minus1; i++)
    {
      if(ColumnWidthInLumaSamples[i] < 256 || RowHeightInLumaSamples[i] <= 64)
        bWarning = true;
    }

    if(bWarning)
    {
      sendWarning("Main Still Picture profile: When a PPS has tiles_enabled_flag is equal to 1, ColumnWidthInLumaSamples[ i ] shall be greater than or equal to "
        "256 for all values of i in the range of 0 to num_tile_columns_minus1, inclusive, and "
        "RowHeightInLumaSamples[ j ] shall be greater than or equal to 64 for all values of j in the range of 0 to "
        "num_tile_rows_minus1, inclusive.", pInfo);
    }
  }
}


void ProfileConformanceAnalyzer::onWarning(const std::string &warning, const HEVC::Parser::Info *pInfo, HEVC::Parser::WarningType)
{
}
