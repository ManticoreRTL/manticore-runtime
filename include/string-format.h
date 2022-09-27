#ifndef __STRING_FORMAT_H__
#define __STRING_FORMAT_H__
// #include <tinyformat.h>
#include <string>
#include <vector>
namespace manticore {
namespace util {

struct Fmt {
  enum class Type { E_STRING, E_HEX, E_DEC, E_BIN };
  virtual std::string toString() const = 0;
  virtual Type type() const = 0;
  virtual ~Fmt() {}
};

struct FmtStringLit : public Fmt {
  FmtStringLit(std::string chars) : chars(chars) {}
  const std::string chars;
  std::string toString() const override { return chars; }
  Type type() const override { return Type::E_STRING; }
};

class FmtNumericLit : public Fmt {

protected:
  FmtNumericLit(const int width, const int digits, const Type tpe,
                const char leading = '0')
      : m_bitwidth(width), m_digits(digits), m_type(tpe), m_leading(leading) {
    int num_values = (width - 1) / 16 + 1;
    std::fill_n(std::back_inserter(m_values), num_values, 0);
  }

public:
  std::string toString() const override;

  Type type() const override { return m_type; }

  void consume(const std::vector<uint16_t> &values) const {
    std::copy(values.begin(), values.end(), m_values.begin());
  }

protected:
  mutable std::vector<uint16_t> m_values;
  const int m_bitwidth;
  const char m_leading;
  const Type m_type;
  const int m_digits;
};

class FmtHexLit : public FmtNumericLit {
public:
  FmtHexLit(const int bitwidth)
      : FmtNumericLit(bitwidth, (bitwidth - 1) / 4 + 1, Type::E_HEX, '0') {}
};
class FmtDecLit : public FmtNumericLit {
public:
  FmtDecLit(const int bitwidth, const int digits, const bool fill_zero = false)
      : FmtNumericLit(bitwidth, digits, Type::E_DEC, fill_zero ? '0' : ' ') {}
};

class FmtBinLit : public FmtNumericLit {
public:
  FmtBinLit(const int bitwidth)
      : FmtNumericLit(bitwidth, bitwidth, Type::E_BIN, '0') {}
};

}; // namespace util
}; // namespace manticore

#endif