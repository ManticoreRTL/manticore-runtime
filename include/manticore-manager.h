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

struct Command final {

  const uint64_t value;
  explicit Command(uint64_t value) : value(value) {}

  static Command start(uint64_t timeout = 0) {
    if (timeout > 0) {
      return Command(timeout | (1L << 63L));
    } else {
      return Command(0);
    }
  }
  static Command resume(uint64_t timeout = 0) {
    if (timeout > 0) {
      return Command((1L << 56L) | timeout | (1L << 63L));
    } else {
      return Command(1L << 56L);
    }
  }

  static Command flushCache() { return Command(2L << 56L); }
};

class ManticoreManager {

public:
  ManticoreManager(const std::shared_ptr<Config> &cfg)
      : m_cfg(cfg), m_run_index(0) {}

  void powerOn();
  void initialize();
  void execute();

private:
  struct ProgramBinary {
    // using Byte = unsigned char;
    const boost::filesystem::path path;
    std::vector<char> content;
    uint64_t base = -1;   // in the global memory (in shorts)
    uint64_t length = -1; // in the global memory (in shorts), i.e., length =
                          // content.size() * 2;
    bool loaded = false;
    ProgramBinary(const boost::filesystem::path &p) : path(p) {}
    void load();
  };
  void initializeMemory();
  uint32_t runProgram(const ProgramBinary &prog, const Command cmd);

  void handleFlush(const std::shared_ptr<FlushException>& exception);

  template <typename R> void write(uint64_t v) {
    m_kernel.write_register(R::base, v);
    if (R::width == 64)
      m_kernel.write_register(R::base + 0x4, v >> 32);
  };

  template <typename R> uint64_t read() {
    uint64_t lo = m_kernel.read_register(R::base);
    if (R::width == 64) {
      uint64_t hi = m_kernel.read_register(R::base + 0x4);
      return lo | (hi << 32UL);
    } else {
      return lo;
    }
  }

  template <class Action>
  inline float timedAction(const std::string &desc, Action action) {
    m_cfg->logger->info("> %s", desc);
    auto start_time = std::chrono::high_resolution_clock::now();
    action();
    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);
    float in_ms = static_cast<float>(elapsed.count()) / 1000.;
    m_cfg->logger->info("> took %.3f ms", in_ms);

    return in_ms;
  }

  xrt::bo m_buffer;
  char* m_host_buffer;
  xrt::device m_dev;
  xrt::ip m_kernel;

  int m_run_index = 0;

  std::shared_ptr<Config> m_cfg;

  std::vector<ProgramBinary> m_binaries;

}; // namespace manticore

} // namespace manticore
#endif
