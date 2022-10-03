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

  opts::options_description opt_desc("Allowed options");

  opt_desc.add_options()
    ("help,h", "show the help message and exit")
    ("xclbin,x", opts::value<boost::filesystem::path>()->required()->value_name("<file>"), "xclbin file (required)")
    ("log,l", opts::value<boost::filesystem::path>()->value_name("<file>"), "redirect runtime logs to a file")
    ("timeout,t", opts::value<uint64_t>()->default_value(0)->value_name("<integer>"), "maximum simulation cycles (0 means no timeout)")
    ("interval", opts::value<uint64_t>()->default_value(100)->value_name("<integer>"), "polling interval")
    ("output,o", opts::value<boost::filesystem::path>()->value_name("<file>")->default_value(boost::filesystem::path()), "redirect simulation output to a file");

  opts::options_description all_opts("Manticore runtime");
  all_opts.add(opt_desc);
  all_opts.add_options()("MANIFEST",opts::value<boost::filesystem::path>()->required()->value_name("<file>"));

  opts::positional_options_description pd;
  pd.add("MANIFEST", 1);

  opts::variables_map vm;
  try {
    opts::store(opts::command_line_parser(argc, argv)
                    .options(all_opts)
                    .positional(pd)
                    .run(),
                vm);

  } catch (opts::error &e) {
    std::cerr << "Failed parsing arguments: " << e.what() << std::endl;
    std::exit(-2);
  }

  if (vm.count("help")) {
    std::cerr << "Usage: " << argv[0] << " [options] <manifest.json>" << std::endl;
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
  auto interval = vm["interval"].as<uint64_t>();
  auto sim_out_path = vm["output"].as<boost::filesystem::path>();
  std::shared_ptr<manticore::FileLogger> logger;
  if (vm.count("log")) {

    logger = std::make_shared<manticore::FileLogger>(vm["log"].as<boost::filesystem::path>());
  }
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
  auto config =
      manticore::Config::load(timeout, interval, xclbin_path, json_path, sim_out_path, logger);

  // check all the files

  for (const auto &fp : config->init_path) {
    checkIsFile(fp);
  }
  checkIsFile(config->program_path);

  // create the device
  auto dev = manticore::ManticoreManager(config);

  dev.powerOn();
  dev.initialize();
  dev.execute();
}