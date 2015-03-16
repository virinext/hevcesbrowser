#include "HevcParser.h"

#include "HevcParserImpl.h"

using namespace HEVC;

Parser::~Parser()
{
}


Parser *Parser::create()
{
  return new HevcParserImpl;
}


void Parser::release(Parser *pparser)
{
  delete pparser;
}
