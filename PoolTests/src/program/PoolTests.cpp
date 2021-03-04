#include <map>
#include <string>

#include <boost/program_options.hpp>
#include "ElementsKernel/ProgramHeaders.h"

#include "PoolTests/FileManager.h"

using boost::program_options::options_description;
using boost::program_options::variable_value;


static Elements::Logging logger = Elements::Logging::getLogger("PoolTests");

class PoolTests : public Elements::Program {

public:

  options_description defineSpecificProgramOptions() override {

    options_description options{};
    return options;
  }

  Elements::ExitCode mainMethod(std::map<std::string, variable_value>& args) override {
    FileManager<int> system_pool;
    //FilePool<fitsfile*> fitsptr_pool;
    //FilePool<CFitsioFile> cfitsio_pool;
    //FilePool<ELFitsFile> elfits_pool;

    std::cout << "Limit: " << system_pool.limit() << std::endl;
    auto fd = system_pool.getAccessor("/etc/hosts");
    std::cout << "Used: " << system_pool.opened() << " " << fd->fd << std::endl;

    return Elements::ExitCode::OK;
  }

};

MAIN_FOR(PoolTests)



