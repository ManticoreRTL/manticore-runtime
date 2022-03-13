#include "config.h"
#include <boost/program_options.hpp>
namespace manticore {

bool Config::parse(int argc, char *argv[]) {
  using namespace boost::program_options;
  options_description opt_desc("Manticore runtime");

  Config cfg;

  opt_desc.add_options()("help,h", "show the help message and exit");
  opt_desc.add_options()("input,i",
                         value<std::vector<boost::filesystem::path>>()
                             ->value_name("PATH1[ PATH2[...]]")
                             ->required()->multitoken(),
                         "Paths of programs to execute in order");
  opt_desc.add_options()(
      "xclbin,x",
      value<boost::filesystem::path>()->value_name("PATH")->required(),
      "Path to the XCLBIN");
  opt_desc.add_options()(
      "timeout", value<unsigned int>()->value_name("N")->default_value(1000),
      "Time out after N cycles, defaults to 1000");

  variables_map vm;
  try {
    store(parse_command_line(argc, argv, opt_desc), vm);
  } catch (error &e) {
    logger->error("Failed parsing arguments: %s", e.what());
    return false;
  }

  if (vm.count("help")) {
    logger->info("%s", opt_desc);
    return false;
  }

  try {
    notify(vm);
  } catch (error &e) {
    logger->error("Error parsing arguments: %s", e.what());
    return false;
  }

  binary_path = vm["input"].as<std::vector<boost::filesystem::path>>();
  xclbin_path = vm["xclbin"].as<boost::filesystem::path>();
  timeout = vm["timeout"].as<unsigned int>();
  return true;
}
} // namespace manticore