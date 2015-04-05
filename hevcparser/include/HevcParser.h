#ifndef HEVC_PARSER_H_
#define HEVC_PARSER_H_

#include "Hevc.h"

#include <cstdint>

namespace HEVC
{
  class Parser
  {
    public:
      struct Info
      {
        std::size_t         m_position;
      };

      class Consumer
      {
        public:
          virtual void onNALUnit(std::shared_ptr<NALUnit> pNALUnit, const Info *pInfo) = 0;
          virtual void onWarning(const std::string &warning, const Info *pInfo) = 0;
      };
      
      virtual ~Parser();
      
      virtual std::size_t process(const uint8_t *pdata, std::size_t size, std::size_t offset = 0) = 0;
      virtual void addConsumer(Consumer *pconsumer) = 0;
      virtual void releaseConsumer(Consumer *pconsumer) = 0;

      static Parser *create();
      static void release(Parser *);
  };
}

#endif
