#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "logger.h"
#include <memory>
#include <vector>
#include <boost/filesystem.hpp>
namespace manticore {

struct Config {

  std::vector<boost::filesystem::path> binary_path;
  boost::filesystem::path xclbin_path;
  uint64_t timeout;

  std::shared_ptr<Logger> logger;
  Config() { logger = std::make_shared<ConsoleLogger>(); }

  bool parse(int argc, char *argv[]);
  bool inputsExists() const {

    for (const auto &bin_file : binary_path) {

      if (!boost::filesystem::exists(bin_file)) {
        logger->error("File does not exists: %s", bin_file);
        return false;
      }
    }
    return true;
  }
};

} // namespace manticore
#endif