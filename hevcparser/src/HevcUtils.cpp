#include "HevcUtils.h"

#include "Hevc.h"

static const uint8_t log2_tab[256]={
        0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
        5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};

uint32_t HEVC::log2(uint32_t k)
{
  uint32_t res = 0;

  if (k & 0xffff0000) {
      k >>= 16;
      res += 16;
  }


  if (k & 0xff00) {
      k >>= 8;
      res += 8;
  }

  res += log2_tab[k];

  return res;
}

uint32_t HEVC::calcNumPocTotalCurr(std::shared_ptr<HEVC::Slice> pslice, std::shared_ptr<HEVC::SPS> psps)
{
  std::size_t NumPocTotalCurr = 0;
  std::size_t currRpsIdx;

  bool UsedByCurrPicLt[16];
  std::size_t num_long_term = pslice -> num_long_term_sps + pslice -> num_long_term_pics;

  for(std::size_t i=0; i < num_long_term; i++)
  {
    if (i < pslice -> num_long_term_sps)
      UsedByCurrPicLt[i] = psps -> used_by_curr_pic_lt_sps_flag[pslice -> lt_idx_sps[i]];
    else
      UsedByCurrPicLt[i] = pslice -> used_by_curr_pic_lt_flag[i];
  }

  if(pslice -> short_term_ref_pic_set_sps_flag)
    currRpsIdx = pslice -> short_term_ref_pic_set_idx;
  else
    currRpsIdx = psps -> num_short_term_ref_pic_sets;

  if(psps -> short_term_ref_pic_set.size() <= currRpsIdx)
  {
    if(currRpsIdx != 0 || pslice->short_term_ref_pic_set_sps_flag)
      return 0;
  }

  ShortTermRefPicSet strps;

  if(currRpsIdx < psps -> short_term_ref_pic_set.size()  )
    strps = psps -> short_term_ref_pic_set[currRpsIdx];
  else
    strps = pslice -> short_term_ref_pic_set;

  for(std::size_t i = 0; i < strps.num_negative_pics; i++)
    if (strps.used_by_curr_pic_s0_flag[i])
      NumPocTotalCurr++;

  for(std::size_t i = 0; i < strps.num_positive_pics; i++)
    if (strps.used_by_curr_pic_s1_flag[i])
      NumPocTotalCurr++;

  for(std::size_t i = 0;i < (pslice -> num_long_term_sps + pslice -> num_long_term_pics); i++)
    if (UsedByCurrPicLt[i])
      NumPocTotalCurr++;

  return NumPocTotalCurr;
}
