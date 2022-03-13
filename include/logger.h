#ifndef __LOGGER_H__
#define __LOGGER_H__
#include <tinyformat.h>
namespace manticore {
class Logger {
public:
  Logger(const std::string &owner) : m_prefix(owner + std::string(":")){};
  Logger() : m_prefix(""){};

  template <typename... Args>
  void warn(const char *fmt, const Args &... args) const {
    do_warn(tfm::format(fmt, args...));
  }
  template <typename... Args>
  void info(const char *fmt, const Args &... args) const {
    do_info(tfm::format(fmt, args...));
  }
  template <typename... Args>
  void error(const char *fmt, const Args &... args) const {
    do_error(tfm::format(fmt, args...));
  }

  virtual ~Logger() {};

protected:
#define ANSI_Red "\033[0;31m"
#define ANSI_Blue "\033[0;34m"
#define ANSI_Yellow "\033[0;33m"
#define ANSI_Reset "\033[0m"
#define COLORED(col)                                                           \
  static inline std::string as##col(const std::string &txt) {                  \
    return tfm::format(ANSI_##col "%s" ANSI_Reset, txt);                       \
  }
  COLORED(Red)
  COLORED(Blue)
  COLORED(Yellow)

  virtual void do_warn(const std::string &msg) const = 0;
  virtual void do_error(const std::string &msg) const = 0;
  virtual void do_info(const std::string &msg) const = 0;

  std::string prefix() const { return m_prefix; }

private:
  const std::string m_prefix;
};

class ConsoleLogger : public Logger {
public:
  ConsoleLogger(const std::string &n) : Logger(n) {}
  ConsoleLogger() : Logger() {}
  ~ConsoleLogger() {}

protected:
  virtual void do_warn(const std::string &msg) const override final;
  virtual void do_error(const std::string &msg) const override final;
  virtual void do_info(const std::string &msg) const override final;
  void do_msg(const std::string &msg) const;
};
} // namespace manticore
#endif
