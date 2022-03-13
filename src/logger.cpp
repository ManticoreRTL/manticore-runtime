#include "logger.h"

namespace manticore {

void ConsoleLogger::do_msg(const std::string& msg) const {
    tfm::printfln("%s", msg);
}
void ConsoleLogger::do_warn(const std::string &msg) const {
   do_msg(tfm::format("[%s] %s %s", asYellow("warning"), prefix(), msg));
}

void ConsoleLogger::do_error(const std::string &msg) const {
  do_msg(tfm::format("[%s] %s %s", asRed("error"), prefix(), msg));

}

void ConsoleLogger::do_info(const std::string &msg) const {
  do_msg(tfm::format("[%s] %s %s", asBlue("info"), prefix(), msg));
}

} // namespace manticore