#include "manticore-manager.h"
#include "manticore-registers.h"
namespace manticore {

void ManticoreManager::powerOn() {

  try {
    float duration = 0.0;
    unsigned int dev_id = 0;
    m_dev = xrt::device(dev_id);

    m_cfg->logger->info("device %s", m_dev.get_info<xrt::info::device::name>());
    timedAction("Creating Kernel", [this]() -> void {
      m_cfg->logger->info("Loading %s", m_cfg->xclbin_path.string());
      auto xclbin = m_dev.load_xclbin(m_cfg->xclbin_path.string());
      m_cfg->logger->info("Constructing kernel");
      m_kernel = xrt::ip(m_dev, xclbin, "ManticoreKernel");
      // m_cfg->logger->info("Allocating buffer");
      // m_buffer = xrt::bo(m_dev, 1 << 20, 1);
    });

  } catch (std::exception &e) {
    m_cfg->logger->error("Could not power on Manticore! %s", e.what());
    std::exit(-3);
  }
}

void ManticoreManager::ProgramBinary::load() {

  auto numBytes = [](boost::filesystem::ifstream &fs) -> size_t {
    auto curr = fs.tellg();
    fs.seekg(0, std::ios::end);
    auto len = fs.tellg();
    fs.seekg(curr);
    return len;
  };
  if (!loaded) {
    boost::filesystem::ifstream fs(path, std::ios::in);
    auto len_bytes = numBytes(fs);
    content.resize(len_bytes);
    fs.read(content.data(), len_bytes);

    assert(len_bytes % 2 == 0);
    assert(sizeof(uint16_t) == 2 * sizeof(char));

    length = len_bytes / sizeof(uint16_t);

    assert(length = content.size() * sizeof(uint16_t));

    fs.close();
  }
}

void ManticoreManager::initializeMemory() {

  // load the binaries into host DRAM
  for (const auto &init : m_cfg->init_path) {
    m_cfg->logger->info("loading %s into memory", init.string());
    m_binaries.emplace_back(init);
    m_binaries.back().load();
  }
  m_cfg->logger->info("loading %s into memory", m_cfg->program_path.string());
  m_binaries.emplace_back(m_cfg->program_path);
  m_binaries.back().load();

  // compute the bound for user and system memory, there might be holes
  // in the middle but that's fine
  uint64_t num_words = m_cfg->global_memory_user_base;
  for (const auto &gm : m_cfg->global_memories) {
    num_words = std::max(gm.base + gm.size, num_words);
  }

  REQUIRE(sizeof(uint16_t) == 2, "something is really wrong!");

  // now we can set the base address of each program binary
  for (auto &binary_program : m_binaries) {
    binary_program.base = num_words;
    num_words += binary_program.length;
  }

  // now we know exactly how much device memory we need.
  uint64_t num_bytes = num_words * sizeof(uint16_t);

  m_cfg->logger->info(
      "Allocating %lu bytes for the global memory buffer object", num_bytes);

  m_buffer = xrt::bo(m_dev, num_bytes, 1);
  m_cfg->logger->info("Mapping device buffer to host");
  // zero out the memory
  auto host_buffer = m_buffer.map<char *>();
  m_cfg->logger->info("Zeroing out host buffer %p (%lu b)", host_buffer,
                      m_buffer.size());
  std::fill(host_buffer, host_buffer + num_bytes, 0);
  // now load all the binaries into the device memory

  for (const auto &binary_program : m_binaries) {
    auto offset = binary_program.base * sizeof(uint16_t);
    m_cfg->logger->info("Writing %s starting at byte offset %lu into memory",
                        binary_program.path.string(), offset);
    std::copy(binary_program.content.begin(), binary_program.content.end(),
              host_buffer + offset);
  }

  m_host_buffer = host_buffer;
  // dma out the content to the device
  timedAction("Syncing global memory",
              [&]() -> void { m_buffer.sync(XCL_BO_SYNC_BO_TO_DEVICE); });
}

void ManticoreManager::initialize() {

  // check the device grid size


  uint32_t dev_info = read<registers::DeviceInfo>();
  uint32_t dimx = (dev_info >> 26) & 63;
  uint32_t dimy = (dev_info >> 20) & 63;
  m_cfg->logger->info("Reading grid information 0x%032x (dimx %d, dimy %d)", dev_info, dimx, dimy);
  m_cfg->logger->info("simulation is compiled for dimx %d, dimy %d", m_cfg->dimx, m_cfg->dimy);
  if (dimx != m_cfg->dimx || dimy != m_cfg->dimy) {
    m_cfg->logger->error(
        "The program is compiled for %dx%d but device is %dx%d (0x%08x)",
        m_cfg->dimx, m_cfg->dimy, dimx, dimy, dev_info);
    std::exit(EXIT_FAILURE);
  }
  // allocate memory and load the programs into it
  initializeMemory();

  // set the pointers
  write<registers::Control>(0);
  write<registers::DramBank0Base>(m_buffer.address());



  // run all the initialization programs
  m_cfg->logger->info("Running initialization programs");
  for (int ix = 0; ix < m_binaries.size() - 1; ix++) {
    uint32_t eid = -1;
    timedAction(tfm::format("initializer %d", ix), [&]() -> void {
      eid = runProgram(m_binaries[ix], Command::start(10));
    });

    if (eid != 0) {
      m_cfg->logger->error("Bad eid %d", eid);
      std::exit(EXIT_FAILURE);
    }
  }
  m_cfg->logger->info("Initialization complete");
}

uint32_t ManticoreManager::runProgram(const ProgramBinary &program,
                                      const Command command) {

  write<registers::ScheduleConfig>(command.value);
  write<registers::GlobalMemoryInstructionBase>(
      program.base);            // set the bootloader pointer
  write<registers::Control>(1); // start the execution;
  bool idle = false;
  std::this_thread::sleep_for(std::chrono::microseconds(100));
  do {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto state = read<registers::Control>();
    idle = ((state >> 2) & 0x01) == 1;
  } while (!idle);
  uint32_t eid = read<registers::ExceptionId>();
  m_cfg->logger->info("Run finished with eid 0x%8x: boot %lu cycles %lu vcycles %lu clock stall %lu cache miss %lu cache hit %lu cache stall %lu",
    read<registers::ExceptionId>(),
    read<registers::BootloaderCycleCount>(),
    read<registers::CycleCount>(),
    read<registers::VirtualCycleCount>(),
    read<registers::ClockStalls>(),
    read<registers::CacheMisses>(),
    read<registers::CacheHits>(),
    read<registers::CacheStalls>()
  );

  
  return eid;
}

void ManticoreManager::execute() {

  const auto &program = m_binaries.back();
  bool failed = false;
  bool resume = false;
  while (true) {
    auto cmd = (resume ? Command::resume(m_cfg->timeout)
                       : Command::start(m_cfg->timeout));
    auto eid = runProgram(program, cmd);

    if (eid > 0xffff) {
      m_cfg->logger->error("Timed out");
      break;
    }
    auto prescription =
        std::find_if(m_cfg->exceptions.begin(), m_cfg->exceptions.end(),
                     [eid](const auto &error) { return error->eid() == eid; });
    if (prescription == m_cfg->exceptions.end()) {
      m_cfg->logger->error("Unknown eid %d!", eid);
      break;
    }

    auto exception = *prescription;
    auto tpe = exception->type();

    if (tpe == ManticoreException::Type::E_FINISH) {
      std::cout << "Got $finish!" << std::endl
                << exception->info() << std::endl;
      break;
    } else if (tpe == ManticoreException::Type::E_ASSERT) {
      std::cerr << "Assertion failed!" << std::endl
                << exception->info() << std::endl;
      failed = true;
      break;
    } else if (tpe == ManticoreException::Type::E_STOP) {
      std::cerr << "Got $stop!" << std::endl << exception->info() << std::endl;
      failed = true;
      break;
    } else if (tpe == ManticoreException::Type::E_FLUSH) {
      resume = true;
      handleFlush(std::dynamic_pointer_cast<FlushException>(exception));
    }
  }
}

void ManticoreManager::handleFlush(
    const std::shared_ptr<FlushException> &exception) {

  // flush the manticore cache
  runProgram(m_binaries.back(), Command::flushCache());

  // now read back the device DDR

  m_buffer.sync(XCL_BO_SYNC_BO_FROM_DEVICE,
                m_cfg->global_memory_user_base / sizeof(uint16_t), 0);

  std::vector<std::vector<uint16_t>> values;
  auto flush_buffer = reinterpret_cast<uint16_t *>(m_host_buffer);
  for (const auto &offsets : exception->offsets()) {
    values.push_back(std::vector<uint16_t>());
    for (const auto offset : offsets) {
      values.back().push_back(flush_buffer[offset]);
    }
  }

  auto message = exception->consume(values);
  if (m_cfg->sim_out.is_open()) {
    m_cfg->sim_out << message << std::endl;
    m_cfg->sim_out.flush();
  } else {
    std::cout << message << std::endl;
  }

  // exception->offsets
}

} // namespace manticore
