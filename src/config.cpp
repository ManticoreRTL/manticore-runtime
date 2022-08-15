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
      "timeout", value<uint64_t>()->value_name("N")->default_value(0),
      "Time out after N cycles, defaults to 0 (i.e., does not time out)");

  opt_desc.add_options()(
    "stop-on-entry", bool_switch(), "stop on entry for waveform debugging"
  );

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
  stop_on_entry = vm["stop-on-entry"].as<bool>();
  timeout = vm["timeout"].as<uint64_t>();
  return true;
}
} // namespace manticore