#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

#include <HevcParser.h>

#include "HEVCInfoWriter.h"
#include "HEVCInfoAltWriter.h"


int main(int argc, char **argv)
{
  namespace po = boost::program_options;
  try 
  {
    po::options_description desc("Options");
    desc.add_options()
      ("help", "produce help message")
      ("altwriter", "user alternative writer")
      ("input,i", po::value<std::string>(), "path to in file")
      ("output,o", po::value<std::string>(), "path to out file")
    ;

    po::variables_map vm;        
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    

    if (vm.count("help")) 
    {
      std::cout << desc << "\n";
      return 0;
    }

    if(!vm.count("input"))
    {
      std::cout << desc << "\n";
      return 1;
    }

    std::ostream *pout = &std::cout;
    std::ofstream fileOut;

    if(vm.count("output"))
    {
      fileOut.open(vm["output"].as<std::string>().c_str());
      if(!fileOut.good())
      {
        std::cerr << "problem with opening file `" << vm["output"].as<std::string>() << "` for writing";
        return 2;
      }
      pout = &fileOut;
    }
    
    std::ifstream in(vm["input"].as<std::string>().c_str(), std::ios_base::binary);
    if(!in.good())
    {
      std::cerr << "problem with opening file `" << vm["input"].as<std::string>() << "` for reading";
      return 2;
    }
    
    in.seekg(0, std::ios::end);
    std::size_t size = in.tellg();
    in.seekg(0, std::ios::beg);
    
    char *pdata = new char[size];
    if(!pdata)
    {
      std::cerr << "Problem with memory allocation. Try to restart computer\n";
      return 3;
    }

    in.read(pdata, size);

    HEVC::Parser *pparser = HEVC::Parser::create();
    HEVCInfoWriter* writer = nullptr;
    if (vm.count("altwriter"))
        writer = new HEVCInfoAltWriter();
    else
        writer = new HEVCInfoWriter();
    pparser -> addConsumer(writer);

    pparser -> process((const uint8_t *)pdata, size);
      
    HEVC::Parser::release(pparser);
    delete [] pdata;
    
    *pout << vm["input"].as<std::string>() << std::endl;
    *pout << "=======================" << std::endl;
    writer->write(*pout);
    delete writer;
  }
  catch(std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
  catch(...) {
    std::cerr << "Exception of unknown type!\n";
    return 1;    
  }

  return 0;
}
