#include "config.h"
#include <boost/program_options.hpp>
#include <json.hpp>

namespace manticore {

std::shared_ptr<Config> Config::load(uint64_t timeout, uint64_t interval,
                                     const boost::filesystem::path &xclbin_path,
                                     const boost::filesystem::path &json_path) {
  using json = nlohmann::json;
  auto fp = std::ifstream(json_path.string(), std::ios::in);
  auto data = json::parse(fp);

  auto config = std::make_shared<manticore::Config>();
  config->interval = interval;
  config->timeout = timeout;
  config->xclbin_path = xclbin_path;

  config->logger->info("Reading json file");
  config->dimx = data["dimx"];
  config->dimy = data["dimy"];
  config->global_memory_user_base = data["base"].get<uint64_t>();
  config->program_path = data["program"].get<boost::filesystem::path>();
  for (const auto &init : data["initializers"]) {
    config->init_path.push_back(init.get<boost::filesystem::path>());
  }
  for (const auto &gmem : data["memories"]) {
    config->global_memories.push_back(GlobalMemory(
        gmem["base"].get<uint64_t>(), gmem["size"].get<uint64_t>()));
  }
  for (const auto &excpt : data["exceptions"]) {

    const std::string type = excpt["type"];
    if (type == "FINISH") {
      config->exceptions.emplace_back(std::make_shared<FinishException>(
          excpt["eid"].get<uint32_t>(), excpt["info"].get<std::string>()));
    } else if (type == "STOP") {
      config->exceptions.emplace_back(std::make_shared<StopException>(
          excpt["eid"].get<uint32_t>(), excpt["info"].get<std::string>()));
    } else if (type == "ASSERT") {
      config->exceptions.emplace_back(std::make_shared<AssertException>(
          excpt["eid"].get<uint32_t>(), excpt["info"].get<std::string>()));
    } else if (type == "FLUSH") {

      std::vector<std::shared_ptr<util::Fmt>> fmt;
      std::vector<std::vector<uint32_t>> offsets;
      auto parseOffsets = [](const json &rec) -> std::vector<uint32_t> {
        std::vector<uint32_t> pointers;
        for (auto &v : rec["offsets"]) {
          pointers.push_back(v.get<uint32_t>());
        }
        return pointers;
      };
      for (const auto &rec : excpt["fmt"]) {
        const auto fmt_type = rec["type"].get<std::string>();
        if (fmt_type == "string") {
          fmt.emplace_back(std::make_shared<util::FmtStringLit>(
              rec["value"].get<std::string>()));
          offsets.push_back(std::vector<uint32_t>());
        } else if (fmt_type == "hex") {
          fmt.emplace_back(
              std::make_shared<util::FmtHexLit>(rec["bitwidth"].get<int>()));
          offsets.push_back(parseOffsets(rec));
        } else if (fmt_type == "dec") {
          fmt.emplace_back(std::make_shared<util::FmtDecLit>(
              rec["bitwidth"].get<int>(), rec["digits"].get<int>(),
              rec["zeros"].get<bool>()));
          offsets.push_back(parseOffsets(rec));
        } else if (fmt_type == "bin") {
          fmt.emplace_back(
              std::make_shared<util::FmtBinLit>(rec["bitwidth"].get<int>()));
          offsets.push_back(parseOffsets(rec));
        } else {
          throw std::runtime_error(
              tfm::format("invalid fmt type %s", fmt_type));
        }
      }

      config->exceptions.emplace_back(std::make_shared<FlushException>(
          excpt["eid"], excpt["info"], fmt, offsets));
    }
  }
  return config;
}

std::string FlushException::consume(
    const std::vector<std::vector<uint16_t>> &values) const {

  // auto fmt_ix = 0;
  REQUIRE(values.size() == m_fmt.size(), "invalid argument! %lu != %lu",
          values.size(), m_fmt.size());

  for (auto fmt_ix = 0; fmt_ix < values.size(); fmt_ix++) {
    switch (m_fmt[fmt_ix]->type()) {
    case util::Fmt::Type::E_STRING:
      REQUIRE(values[fmt_ix].size() == 0, "invalid argument! %lu != 0",
              values[fmt_ix].size());
      break;
      // do nothing
    case util::Fmt::Type::E_BIN:
    case util::Fmt::Type::E_DEC:
    case util::Fmt::Type::E_HEX:
      std::dynamic_pointer_cast<util::FmtNumericLit>(m_fmt[fmt_ix])
          ->consume(values[fmt_ix]);
      break;
    default:
      throw std::runtime_error("Invalid format type!");
      break;
    }
  }
  auto builder = std::stringstream("");

  for (const auto &fmt : m_fmt) {
    builder << fmt->toString();
  }
  return builder.str();
}
} // namespace manticore