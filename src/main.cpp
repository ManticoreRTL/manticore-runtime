#include "config.h"
#include "manticore-manager.h"
#include <chrono>
#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {


  auto cfg = std::make_shared<manticore::Config>();
  if (!cfg->parse(argc, argv)) {
    std::exit(-1);
  }

  std::cout << cfg->binary_path.size() << std::endl;
  if (!cfg->inputsExists()) {
    std::exit(-2);
  }

  manticore::ManticoreManager dev(cfg);

  dev.powerOn();
  if (cfg->stop_on_entry) {
    cfg->logger->info("Press a key to continue!");
    std::cin.get();
  }

  dev.runAll();

}