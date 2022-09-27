#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "logger.h"
#include "string-format.h"
#include <boost/filesystem.hpp>
#include <memory.h>
#include <memory>
#include <vector>
namespace manticore {

struct ManticoreException {
  enum class Type { E_FINISH, E_STOP, E_ASSERT, E_FLUSH };

  virtual Type type() const = 0;
  virtual ~ManticoreException() {}
  virtual uint32_t eid() const = 0;
  virtual bool terminating() const = 0;
};

class TerminatingException : public ManticoreException {
public:
  Type type() const override final { return m_type; }
  bool terminating() const override final { return true; }
  uint32_t eid() const override final { return m_id; }

protected:
  TerminatingException(uint32_t id, Type tpe, const std::string &info)
      : m_type(tpe), m_id(id), m_info(info) {}
  const Type m_type;
  const std::string m_info;
  const uint32_t m_id;
};
class FinishException : public TerminatingException {
public:
  FinishException(uint32_t id, const std::string &info)
      : TerminatingException(id, Type::E_FINISH, info) {}
};
class StopException : public TerminatingException {
public:
  StopException(uint32_t id, const std::string &info)
      : TerminatingException(id, Type::E_STOP, info) {}
};
class AssertException : public TerminatingException {
public:
  AssertException(uint32_t id, const std::string &info)
      : TerminatingException(id, Type::E_ASSERT, info) {}
};

class FlushException : public ManticoreException {
public:
  FlushException(uint32_t id, const std::string &info,
                 const std::vector<std::shared_ptr<util::Fmt>> &fmt,
                 const std::vector<std::vector<uint32_t>> &offsets)
      : m_id(id), m_info(info), m_fmt(fmt), m_offsets(offsets) {}
  Type type() const override { return Type::E_FLUSH; }
  uint32_t eid() const override { return m_id; }
  bool terminating() const override { return false; }
  const std::vector<std::vector<uint32_t>> &offsets() const {
    return m_offsets;
  }
  std::string consume(const std::vector<std::vector<uint16_t>> &values) const;

private:
  const std::vector<std::shared_ptr<util::Fmt>> m_fmt;
  const std::vector<std::vector<uint32_t>> m_offsets;
  const uint32_t m_id;
  const std::string m_info;
};
struct Config {

  std::vector<boost::filesystem::path> init_path;
  boost::filesystem::path program_path;
  int dimx, dimy;
  std::vector<std::shared_ptr<ManticoreException>> exceptions;

  boost::filesystem::path xclbin_path;
  uint64_t timeout;

  std::shared_ptr<Logger> logger;
  Config() { logger = std::make_shared<ConsoleLogger>(); }

  static std::shared_ptr<Config> load(uint64_t timeout,
                     const boost::filesystem::path &xclbin_path,
                     const boost::filesystem::path &json_path);

};

} // namespace manticore
#endif