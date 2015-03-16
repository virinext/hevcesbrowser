#ifndef HEVC_PARSER_IMPL
#define HEVC_PARSER_IMPL



#include "HevcParser.h"
#include "BitstreamReader.h"

#include <map>
#include <list>
#include <memory>

namespace HEVC
{
  class HevcParserImpl: public Parser
  {
    public:
      virtual std::size_t process(const uint8_t *pdata, std::size_t size, std::size_t offset = 0);  
      
      virtual void addConsumer(Consumer *pconsumer);
      virtual void releaseConsumer(Consumer *pconsumer);
      
    protected:
      void processNALUnit(const uint8_t *pdata, std::size_t size, const Parser::Info &info);
      NALUnitType processNALUnitHeader(BitstreamReader &bs);
      void processVPS(std::shared_ptr<VPS> pvps, BitstreamReader &bs, const Parser::Info &info);
      void processSPS(std::shared_ptr<SPS> psps, BitstreamReader &bs, const Parser::Info &info);
      void processPPS(std::shared_ptr<PPS> ppps, BitstreamReader &bs, const Parser::Info &info);
      void processSlice(std::shared_ptr<Slice> pslice, BitstreamReader &bs, const Parser::Info &info);
      void processSliceHeader(std::shared_ptr<Slice> pslice, BitstreamReader &bs, const Parser::Info &info);
      void processSliceData(std::shared_ptr<Slice> pslice, BitstreamReader &bs, const Parser::Info &info);
      void processProfileTierLevel(std::shared_ptr<SPS> psps, BitstreamReader &bs, const Parser::Info &info);
      
      std::map<uint32_t, std::shared_ptr<VPS> >          m_vpsMap;
      std::map<uint32_t, std::shared_ptr<SPS> >          m_spsMap;
      std::map<uint32_t, std::shared_ptr<PPS> >          m_ppsMap;

      std::list<Consumer *>          m_consumers;
  };
}

#endif
