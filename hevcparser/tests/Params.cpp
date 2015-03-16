#include "Params.h"

std::string getSourceDir()
{
  std::string fileName = __FILE__;
  std::size_t pos = fileName.find_last_of("\\/");

  return fileName.substr(0, pos + 1);
}
