#ifndef TYPES_H_
#define TYPES_H_

typedef std::map<uint32_t, std::shared_ptr<HEVC::VPS> > VPSMap;
Q_DECLARE_METATYPE(VPSMap);

typedef std::map<uint32_t, std::shared_ptr<HEVC::SPS> > SPSMap;
Q_DECLARE_METATYPE(SPSMap);

typedef std::map<uint32_t, std::shared_ptr<HEVC::PPS> > PPSMap;
Q_DECLARE_METATYPE(PPSMap);

typedef HEVC::Parser::Info ParserInfo;
Q_DECLARE_METATYPE(ParserInfo);


#endif
