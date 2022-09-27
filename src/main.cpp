#include "config.h"
#include "manticore-manager.h"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <memory>
int main(int argc, char *argv[]) {

  // parse the arguments

  namespace opts = boost::program_options;

  opts::options_description opt_desc("Manticore runtime");

  opt_desc.add_options()("help,h", "show the help message and exit")(
      "xclbin,x", opts::value<boost::filesystem::path>()->required(),
      "xclbin file (required)")(
      "timeout,t", opts::value<uint64_t>()->default_value(0),
      "maximum simulation cycles (0 means no timeout)")(
      "MANIFEST", opts::value<boost::filesystem::path>()->required());

  opts::positional_options_description pd;
  pd.add("MANIFEST", 1);

  opts::variables_map vm;
  try {
    opts::store(opts::command_line_parser(argc, argv)
                    .options(opt_desc)
                    .positional(pd)
                    .run(),
                vm);

  } catch (opts::error &e) {
    std::cerr << "Failed parsing arguments: " << e.what() << std::endl;
    std::exit(-2);
  }

  if (vm.count("help")) {
    std::cerr << opt_desc << std::endl;
    std::exit(-1);
  }

  try {
    opts::notify(vm);
  } catch (opts::error &e) {
    std::cerr << "Failed parsing arguments: " << e.what() << std::endl;
    std::exit(-2);
  }
  auto xclbin_path = vm["xclbin"].as<boost::filesystem::path>();
  auto timeout = vm["timeout"].as<uint64_t>();
  auto json_path = vm["MANIFEST"].as<boost::filesystem::path>();

  auto checkIsFile = [](const auto &path) {
    if (boost::filesystem::is_regular_file(path) == false) {
      std::cerr << "Could not find" << boost::filesystem::canonical(path)
                << std::endl;
      std::exit(-3);
    }
  };

  checkIsFile(xclbin_path);
  checkIsFile(json_path);
  // create a config by loading the json file
  auto config = manticore::Config::load(timeout, xclbin_path, json_path);

  // check all the files

  for (const auto &fp : config->init_path) {
    checkIsFile(fp);
  }
  checkIsFile(config->program_path);

  // create the device
  auto dev = manticore::ManticoreManager(config);

  dev.powerOn();
  // std::cout << cfg->binary_path.size() << std::endl;
  // if (!cfg->inputsExists()) {
  //   std::exit(-2);
  // }

  // manticore::ManticoreManager dev(cfg);

  // dev.powerOn();
  // if (cfg->stop_on_entry) {
  //   cfg->logger->info("Press a key to continue!");
  //   std::cin.get();
  // }

  // dev.runAll();
}