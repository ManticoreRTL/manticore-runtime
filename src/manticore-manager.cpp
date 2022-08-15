#include "manticore-manager.h"

namespace manticore {

void ManticoreManager::powerOn() {

  try {
    float duration = 0.0;
    unsigned int dev_id = 0;
    m_dev = xrt::device(dev_id);

    m_cfg->logger->info("%s", m_dev.get_info<xrt::info::device::name>());
    timedAction("Creating Kernel", [this]() -> void {
      m_cfg->logger->info("Loading %s", m_cfg->xclbin_path.string());
      auto xclbin = m_dev.load_xclbin(m_cfg->xclbin_path.string());
      m_cfg->logger->info("Constructing kernel");
      m_kernel = xrt::ip(m_dev, xclbin, "ManticoreKernel");
      m_cfg->logger->info("Allocating memory");
      m_buffer = xrt::bo(m_dev, 1 << 25, 1);
    });

  } catch (std::exception &e) {
    m_cfg->logger->error("Could not power on Manticore! %s", e.what());
    std::exit(-3);
  }
}

void ManticoreManager::run(const boost::filesystem::path &path) {

  auto numBytes = [](boost::filesystem::ifstream &fs) -> size_t {
    auto curr = fs.tellg();
    fs.seekg(0, std::ios::end);
    auto len = fs.tellg();
    fs.seekg(curr);
    return len;
  };

  if (path.extension() == ".dat") {

    boost::filesystem::ifstream fs(path, std::ios::in);
    auto len = numBytes(fs);
    if (len > m_buffer.size()) {
      m_cfg->logger->error("Not enough memory to load program %s",
                           path.string());
      return;
    }
    timedAction("loading ascii file into DRAM", [&]() -> void {
      std::string line;
      auto buffer = m_buffer.map<uint16_t *>();
      unsigned int offset = 0;
      while (std::getline(fs, line) && offset < m_buffer.size() / 2) {
        std::size_t pos = 0;
        const unsigned int i = std::stoi(line, &pos, 2);
        std::cout << i << std::endl;
        if (pos != 16) {
          fs.close();
          throw std::runtime_error("Bad line " + line);
        }
        buffer[offset] = static_cast<uint16_t>(i);
        offset++;
      }
      fs.close();
      m_buffer.sync(XCL_BO_SYNC_BO_TO_DEVICE, offset * 2, 0);
    });
  } else {
    boost::filesystem::ifstream fs(path, std::ios::binary);
    auto mem = m_buffer.map<char *>();
    auto length = numBytes(fs);
    if (m_buffer.size() < length) {
      m_cfg->logger->error("Not enough memory to load program %s",
                           path.string());
      return;
    }
    timedAction("Loading program to DRAM", [&]() -> void {
      fs.read(mem, length);
      m_buffer.sync(XCL_BO_SYNC_BO_TO_DEVICE, length, 0);
    });
  }

  timedAction("Starting kernel", [&]() -> void {
    write<GMEM_BASE>(0);
    write<PTR0>(m_buffer.address());
    uint64_t timeout = m_cfg->timeout;
    write<SCHED_CONFIG>(Commands::START(m_cfg->timeout));
    write<CTRL>(1);
    bool idle = false;
    do {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      auto state = m_kernel.read_register(0x00);
      idle = ((state >> 2) & 0x01) == 1;
    } while (!idle);
    m_cfg->logger->info("Boot: %d", read<BOOTLOADER_CYCLES>());
    m_cfg->logger->info("cycles: %d", read<EXEC_CYCLES>());
    m_cfg->logger->info("vcycles: %d", read<VCYCLES>());
    m_cfg->logger->info("exception_id: %d", read<EXCEPTION_ID>());
  });
}

void ManticoreManager::runAll() {
  for (const auto &bin_path : m_cfg->binary_path) {
    m_cfg->logger->info("Starting %s", bin_path.string());
    run(bin_path);
  }
}

} // namespace manticore
