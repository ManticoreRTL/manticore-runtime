#include "string-format.h"
#include <InfInt.h>
#include <sstream>
#include <tinyformat.h>
#include <bitset>
namespace manticore::util {

std::string FmtNumericLit::toString() const {

  auto builder = std::stringstream("");

  if (m_type == Type::E_DEC) {
    // determine the total number of digits based on the bit width
    InfInt v = 0;
    InfInt shift = 1;
    for (auto it = m_values.begin(); it != m_values.end(); it++) {
      InfInt tv = *it;
      tv *= shift;
      v += tv;
      shift *= (1 << 16);
    }
    builder << v;
  } else if (m_type == Type::E_BIN) {

    for (auto it = m_values.rbegin(); it != m_values.rend(); it++) {
        builder << std::bitset<16>(*it);
    }

  } else if (m_type == Type::E_HEX) {

    for (auto it = m_values.rbegin(); it != m_values.rend(); it++) {
        std::stringstream digits;
        digits << std::hex << (*it);
        for (int i = 0; i < (4 - digits.str().size()); i ++) {
            builder << m_leading;
        }
        builder << std::hex << (*it);
    }

  }

  auto raw_str = builder.str();
  if (raw_str.size() < m_digits) {
    std::stringstream padding;
    for (int i = 0; i < (m_digits - raw_str.size()); i++) {
        padding << m_leading;
    }
    padding << raw_str;
    return padding.str();
  } else if (raw_str.size() == m_digits) {
    return raw_str;
  } else {
    return raw_str.substr(raw_str.size() - m_digits);
  }
}
}; // namespace manticore::util