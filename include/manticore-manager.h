#ifndef __MANTICORE_MANAGER_H__
#define __MANTICORE_MANAGER_H__
#include "config.h"
#include <chrono>
#include <experimental/xrt_error.h>
#include <experimental/xrt_ip.h>
#include <memory>
#include <xrt/xrt_bo.h>
#include <xrt/xrt_device.h>
#include <xrt/xrt_kernel.h>
#include <xrt/xrt_uuid.h>

namespace manticore {

class ManticoreManager {

public:
  ManticoreManager(const std::shared_ptr<Config> &cfg)
      : m_cfg(cfg), m_run_index(0) {}

  void powerOn();

  void run(const boost::filesystem::path &path);
  void runAll();

private:
  struct Address {
    static constexpr int GMEM_OFFSET = 0x30;
    static constexpr int GMEM_PTR = 0x60;
  };
  template <class Action>
  inline float timedAction(const std::string &desc, Action action) {
    m_cfg->logger->info("==> %s", desc);
    auto start_time = std::chrono::high_resolution_clock::now();
    action();
    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);
    float in_ms = static_cast<float>(elapsed.count()) / 1000.;
    m_cfg->logger->info("==| took %.3f ms", in_ms);
    return in_ms;
  }

  xrt::bo m_buffer;
  xrt::device m_dev;
  xrt::ip m_kernel;

  int m_run_index = 0;

  std::shared_ptr<Config> m_cfg;
};

} // namespace manticore
#endif
