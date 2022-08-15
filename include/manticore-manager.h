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


struct Commands final {


  static uint64_t START(uint64_t timeout = 0) {
    if (timeout > 0) {
      return timeout | (1L << 63L);
    } else {
      return 0;
    }
  }
  static uint64_t RESUME(uint64_t timeout = 0) {
    if (timeout > 0) {
      return ((1L << 56L) | timeout | (1L << 63L));
    } else {
      return 1L << 56L;
    }
  }

  static uint64_t CACHE_FLUSH() {
    return (2L << 56L);
  }
};

class ManticoreManager {

public:
  ManticoreManager(const std::shared_ptr<Config> &cfg)
      : m_cfg(cfg), m_run_index(0) {}

  void powerOn();

  void run(const boost::filesystem::path &path);
  void runAll();

private:
#define MREG(N, B, W)                                                          \
  struct N {                                                                   \
    static constexpr int base = B;                                             \
    static constexpr int width = 64;                                           \
  };

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
  MREG(CTRL, 0x00, 32);
  MREG(SCHED_CONFIG, 0x10, 64);
  MREG(VCD_LOG_BASE, 0x1c, 64);
  MREG(VCD_SYM_BASE, 0x28, 64);
  MREG(GMEM_BASE, 0x34, 64);
  MREG(PTR3, 0x40, 64);
  MREG(PTR2, 0x4C, 64);
  MREG(PTR1, 0x58, 64);
  MREG(PTR0, 0x64, 64);
  MREG(EXEC_CYCLES, 0x70, 64);
  MREG(EXCEPTION_ID, 0x7c, 32);
  MREG(BOOTLOADER_CYCLES, 0x84, 32);
  MREG(VCYCLES, 0x8c, 64);

  /**
   * @brief
   *
   *  0x10 -> schedule_config (64)
   *  0x1c -> value_change_log_base (64)
   *  0x28 -> value_change_symbol_table_base (64)
   *  0x34 -> global_memory_instruction_base (64)
   *  0x40 -> pointer_3 (64)
   *  0x4c -> pointer_2 (64)
   *  0x58 -> pointer_1 (64)
   *  0x64 -> pointer_0 (64)
   *  0x70 -> execution_cycles (64)
   *  0x7c -> exception_id (32)
   *  0x84 -> bootloader_cycles (32)
   *  0x8c -> virtual_cycles (64)
   *  Writeable address range 16 -> 104 (total 23)
   */
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
}; // namespace manticore

} // namespace manticore
#endif
