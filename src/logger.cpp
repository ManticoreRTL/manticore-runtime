#include "logger.h"

namespace manticore {

void ConsoleLogger::do_msg(const std::string &msg) const {
  tfm::printfln("%s", msg);
}
void ConsoleLogger::do_warn(const std::string &msg) const {
  do_msg(tfm::format("[%s] %s %s", asYellow("warn"), prefix(), msg));
}

void ConsoleLogger::do_error(const std::string &msg) const {
  do_msg(tfm::format("[%s] %s %s", asRed("error"), prefix(), msg));
}

void ConsoleLogger::do_info(const std::string &msg) const {
  do_msg(tfm::format("[%s] %s %s", asBlue("info"), prefix(), msg));
}

void FileLogger::do_warn(const std::string &msg) const {
  m_ofs << "[warn] " << msg << std::endl;
  m_ofs.flush();
}

void FileLogger::do_info(const std::string &msg) const {

  m_ofs << "[info] " << msg << std::endl;
  if (m_auto_flush) {
    m_ofs.flush();
  }
}

void FileLogger::do_error(const std::string &msg) const {

  m_ofs << "[error] " << msg << std::endl;
  m_ofs.flush();
}

FileLogger::FileLogger(const boost::filesystem::path &p, bool auto_flush)
    : m_auto_flush(auto_flush) {
  m_ofs.open(p, std::ios::out);

}

FileLogger::~FileLogger() {
  m_ofs.close();
}

} // namespace manticore