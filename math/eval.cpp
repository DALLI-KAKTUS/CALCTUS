#include "math/eval.h"

#include <cctype>
#include <cstdlib>
#include <cstring>

extern "C" {
#include "libbf.h"
}

namespace calctus::math {
namespace {

constexpr limb_t kPrecision = 34;  // decimal128 düzeyinde anlamlı basamak
constexpr limb_t kBinaryPrecision = 160;
constexpr bf_flags_t kFlags = BF_RNDN | BF_FLAG_EXT_EXP;

void* resize(void*, void* ptr, size_t size) {
  return size ? std::realloc(ptr, size) : (std::free(ptr), nullptr);
}

struct Value {
  bfdec_t n{};
  explicit Value(bf_context_t* ctx) { bfdec_init(ctx, &n); }
  ~Value() { bfdec_delete(&n); }
  Value(const Value&) = delete;
  Value& operator=(const Value&) = delete;
};

class Parser {
 public:
  Parser(bf_context_t* ctx, const char* text, const char* ans, AngleUnit angle, const char* x)
      : ctx_(ctx), p_(text), ans_(ans), angle_(angle), x_(x) {}

  Error run(bfdec_t* result) {
    if (!p_ || !*p_) return Error::Syntax;
    expression(result);
    spaces();
    if (error_ == Error::None && *p_) error_ = Error::Syntax;
    return error_;
  }

 private:
  void spaces() { while (*p_ == ' ') ++p_; }
  bool take(char c) {
    spaces();
    if (*p_ != c) return false;
    ++p_;
    return true;
  }
  void status(int st) {
    if (st & BF_ST_MEM_ERROR) error_ = Error::Memory;
    else if (st & BF_ST_DIVIDE_ZERO) error_ = Error::DivideZero;
    else if (st & BF_ST_INVALID_OP) error_ = Error::Domain;
    else if (st & (BF_ST_OVERFLOW | BF_ST_UNDERFLOW)) error_ = Error::TooLarge;
  }
  void parsed_value(bfdec_t* out, const char* text) {
    if (!text) { error_ = Error::Variable; return; }
    const char* cursor = text;
    if (*cursor == '+' || *cursor == '-') ++cursor;
    bool digits = false;
    while (*cursor >= '0' && *cursor <= '9') { ++cursor; digits = true; }
    if (*cursor == '.') {
      ++cursor;
      while (*cursor >= '0' && *cursor <= '9') { ++cursor; digits = true; }
    }
    if (!digits) { error_ = Error::Syntax; return; }
    if (*cursor == 'e' || *cursor == 'E') {
      ++cursor;
      if (*cursor == '+' || *cursor == '-') ++cursor;
      if (*cursor < '0' || *cursor > '9') { error_ = Error::Syntax; return; }
      unsigned magnitude = 0;
      while (*cursor >= '0' && *cursor <= '9') {
        magnitude = magnitude * 10 + static_cast<unsigned>(*cursor++ - '0');
        if (magnitude > 10000) { error_ = Error::TooLarge; return; }
      }
    }
    if (*cursor || cursor - text > 127) { error_ = Error::Syntax; return; }
    const char* end = nullptr;
    status(bfdec_atof(out, text, &end, kPrecision, kFlags | BF_ATOF_NO_NAN_INF));
    if (error_ == Error::None && (!end || end == text || *end)) error_ = Error::Syntax;
  }
  void transcendental(bfdec_t* out, const bfdec_t* arg, char kind) {
    bf_t input{}, result{}, pi{}, divisor{}, scaled{};
    bf_init(ctx_, &input);
    bf_init(ctx_, &result);
    bf_init(ctx_, &pi);
    bf_init(ctx_, &divisor);
    bf_init(ctx_, &scaled);
    status(bfdec_to_f(&input, arg, kBinaryPrecision, kFlags));
    if (error_ == Error::None && (kind == 's' || kind == 'c' || kind == 't') &&
        angle_ != AngleUnit::Radian) {
      status(bf_const_pi(&pi, kBinaryPrecision, kFlags));
      status(bf_set_ui(&divisor, angle_ == AngleUnit::Degree ? 180 : 200));
      if (error_ == Error::None)
        status(bf_mul(&scaled, &input, &pi, kBinaryPrecision, kFlags));
      if (error_ == Error::None)
        status(bf_div(&input, &scaled, &divisor, kBinaryPrecision, kFlags));
    }
    if (error_ == Error::None) {
      int st = 0;
      switch (kind) {
        case 's': st = bf_sin(&result, &input, kBinaryPrecision, kFlags); break;
        case 'c': st = bf_cos(&result, &input, kBinaryPrecision, kFlags); break;
        case 't': st = bf_tan(&result, &input, kBinaryPrecision, kFlags); break;
        case 'e': st = bf_exp(&result, &input, kBinaryPrecision, kFlags); break;
        case 'l': st = bf_log(&result, &input, kBinaryPrecision, kFlags); break;
        case 'g': {
          bf_t ten{}, logten{};
          bf_init(ctx_, &ten);
          bf_init(ctx_, &logten);
          status(bf_set_ui(&ten, 10));
          if (error_ == Error::None) status(bf_log(&logten, &ten, kBinaryPrecision, kFlags));
          if (error_ == Error::None) {
            status(bf_log(&result, &input, kBinaryPrecision, kFlags));
            if (error_ == Error::None) {
              bf_t quotient{};
              bf_init(ctx_, &quotient);
              status(bf_div(&quotient, &result, &logten, kBinaryPrecision, kFlags));
              if (error_ == Error::None) status(bf_set(&result, &quotient));
              bf_delete(&quotient);
            }
          }
          bf_delete(&logten);
          bf_delete(&ten);
          break;
        }
      }
      status(st);
      if (error_ == Error::None)
        status(bfdec_from_f(out, &result, kPrecision, kFlags));
    }
    bf_delete(&scaled);
    bf_delete(&divisor);
    bf_delete(&pi);
    bf_delete(&result);
    bf_delete(&input);
  }
  void expression(bfdec_t* out) {
    term(out);
    while (error_ == Error::None) {
      spaces();
      char op = *p_;
      if (op != '+' && op != '-') break;
      ++p_;
      Value rhs(ctx_), result(ctx_);
      term(&rhs.n);
      if (error_ != Error::None) return;
      status(op == '+' ? bfdec_add(&result.n, out, &rhs.n, kPrecision, kFlags)
                       : bfdec_sub(&result.n, out, &rhs.n, kPrecision, kFlags));
      status(bfdec_set(out, &result.n));
    }
  }
  void term(bfdec_t* out) {
    unary(out);
    while (error_ == Error::None) {
      spaces();
      char op = *p_;
      if (op != '*' && op != '/') break;
      ++p_;
      Value rhs(ctx_), result(ctx_);
      unary(&rhs.n);
      if (error_ != Error::None) return;
      status(op == '*' ? bfdec_mul(&result.n, out, &rhs.n, kPrecision, kFlags)
                       : bfdec_div(&result.n, out, &rhs.n, kPrecision, kFlags));
      status(bfdec_set(out, &result.n));
    }
  }
  void unary(bfdec_t* out) {
    if (++unary_depth_ > 16) { error_ = Error::TooLarge; --unary_depth_; return; }
    if (take('+')) unary(out);
    else if (take('-')) {
      unary(out);
      if (error_ == Error::None) bfdec_neg(out);
    } else power(out);
    --unary_depth_;
  }
  void power(bfdec_t* out) {
    primary(out);
    if (error_ != Error::None || !take('^')) return;
    Value exponent_value(ctx_), integer_value(ctx_);
    unary(&exponent_value.n); // sağdan birleşir: 2^3^2 = 2^(3^2)
    if (error_ != Error::None) return;
    int exponent = 0;
    const int st = bfdec_get_int32(&exponent, &exponent_value.n);
    if (st || exponent < -100 || exponent > 100) {
      error_ = Error::TooLarge;
      return;
    }
    status(bfdec_set_si(&integer_value.n, exponent));
    const bool integer = bfdec_cmp(&integer_value.n, &exponent_value.n) == 0;
    if (integer && exponent == 0 && bfdec_is_zero(out)) {
      error_ = Error::Domain;
      return;
    }
    if (!integer) {
      bf_t base{}, exp{}, result{};
      bf_init(ctx_, &base);
      bf_init(ctx_, &exp);
      bf_init(ctx_, &result);
      status(bfdec_to_f(&base, out, kBinaryPrecision, kFlags));
      if (error_ == Error::None)
        status(bfdec_to_f(&exp, &exponent_value.n, kBinaryPrecision, kFlags));
      if (error_ == Error::None) status(bf_pow(&result, &base, &exp, kBinaryPrecision, kFlags));
      if (error_ == Error::None) status(bfdec_from_f(out, &result, kPrecision, kFlags));
      bf_delete(&result);
      bf_delete(&exp);
      bf_delete(&base);
      return;
    }
    Value result(ctx_);
    const unsigned magnitude = static_cast<unsigned>(exponent < 0 ? -exponent : exponent);
    status(bfdec_pow_ui(&result.n, out, magnitude));
    if (error_ != Error::None) return;
    if (exponent < 0) {
      Value one(ctx_), quotient(ctx_);
      status(bfdec_set_ui(&one.n, 1));
      status(bfdec_div(&quotient.n, &one.n, &result.n, kPrecision, kFlags));
      status(bfdec_set(out, &quotient.n));
    } else {
      status(bfdec_set(out, &result.n));
    }
  }
  void primary(bfdec_t* out) {
    if (++depth_ > 16) { error_ = Error::TooLarge; --depth_; return; }
    spaces();
    if (take('(')) {
      expression(out);
      if (error_ == Error::None && !take(')')) error_ = Error::Syntax;
    } else if (std::strncmp(p_, "sqrt(", 5) == 0 ||
               std::strncmp(p_, "sin(", 4) == 0 ||
               std::strncmp(p_, "cos(", 4) == 0 ||
               std::strncmp(p_, "tan(", 4) == 0 ||
               std::strncmp(p_, "exp(", 4) == 0 ||
               std::strncmp(p_, "log(", 4) == 0 ||
               std::strncmp(p_, "ln(", 3) == 0) {
      const char kind = p_[0] == 's' && p_[1] == 'q' ? 'r' :
                        p_[0] == 'l' && p_[1] == 'n' ? 'l' :
                        p_[0] == 'l' ? 'g' : p_[0];
      p_ = std::strchr(p_, '(') + 1;
      Value arg(ctx_);
      expression(&arg.n);
      if (error_ == Error::None && !take(')')) error_ = Error::Syntax;
      if (error_ == Error::None) {
        if (kind == 'r') status(bfdec_sqrt(out, &arg.n, kPrecision, kFlags));
        else transcendental(out, &arg.n, kind);
      }
    } else if (std::strncmp(p_, "Ans", 3) == 0) {
      p_ += 3;
      parsed_value(out, ans_);
    } else if (*p_ == 'x') {
      ++p_;
      parsed_value(out, x_);
    } else if (std::strncmp(p_, "pi", 2) == 0) {
      p_ += 2;
      bf_t pi{};
      bf_init(ctx_, &pi);
      status(bf_const_pi(&pi, kBinaryPrecision, kFlags));
      if (error_ == Error::None) status(bfdec_from_f(out, &pi, kPrecision, kFlags));
      bf_delete(&pi);
    } else if (std::isdigit(static_cast<unsigned char>(*p_)) || *p_ == '.') {
      const char* start = p_;
      bool digits = false;
      while (std::isdigit(static_cast<unsigned char>(*p_))) { ++p_; digits = true; }
      if (*p_ == '.') {
        ++p_;
        while (std::isdigit(static_cast<unsigned char>(*p_))) { ++p_; digits = true; }
      }
      if (!digits) { error_ = Error::Syntax; --depth_; return; }
      if (*p_ == 'e' || *p_ == 'E') {
        ++p_;
        if (*p_ == '+' || *p_ == '-') ++p_;
        unsigned exp = 0;
        if (!std::isdigit(static_cast<unsigned char>(*p_))) error_ = Error::Syntax;
        while (std::isdigit(static_cast<unsigned char>(*p_))) {
          exp = exp * 10 + static_cast<unsigned>(*p_++ - '0');
          if (exp > 99) { error_ = Error::TooLarge; break; }
        }
      }
      if (error_ == Error::None) {
        // libbf'nin okuyucusu kendi ayıracını döndürür; ayrıştırdığımız sayıyla eşleşmeli.
        const char* end = nullptr;
        status(bfdec_atof(out, start, &end, kPrecision, kFlags));
        if (end != p_) error_ = Error::Syntax;
      }
    } else {
      error_ = Error::Syntax;
    }
    --depth_;
  }

  bf_context_t* ctx_;
  const char* p_;
  const char* ans_;
  AngleUnit angle_;
  const char* x_;
  Error error_ = Error::None;
  int depth_ = 0;
  int unary_depth_ = 0;
};

}  // namespace

Error evaluate(const char* expression, const char* ans, NumberFormat format,
               char* out, size_t capacity, AngleUnit angle, const char* x) {
  if (!out || !capacity) return Error::TooLarge;
  out[0] = '\0';
  if (!expression || std::strlen(expression) > 96 || !ans) return Error::TooLarge;
  bf_context_t ctx;
  bf_context_init(&ctx, resize, nullptr);
  Error error;
  {
    Value value(&ctx);
    Parser parser(&ctx, expression, ans, angle, x);
    error = parser.run(&value.n);
    if (error == Error::None) {
      const bf_flags_t flags = kFlags | (format == NumberFormat::Scientific
                                             ? BF_FTOA_FORCE_EXP : BF_FTOA_FORMAT_FREE);
      char* text = bfdec_ftoa(nullptr, &value.n,
                              format == NumberFormat::Auto ? kPrecision : 16,
                              format == NumberFormat::Scientific
                                  ? flags | BF_FTOA_FORMAT_FIXED
                                  : format == NumberFormat::Decimal
                                      ? kFlags | BF_FTOA_FORMAT_FIXED : flags);
      if (!text) error = Error::Memory;
      else {
        size_t length = std::strlen(text);
        if (length >= capacity) error = Error::TooLarge;
        else std::memcpy(out, text, length + 1);
        bf_free(&ctx, text);
      }
    }
  }
  bf_context_end(&ctx);
  return error;
}

const char* error_text(Error error) {
  switch (error) {
    case Error::None: return "";
    case Error::Syntax: return "Sözdizimi hatası";
    case Error::Variable: return "x değeri tanımsız";
    case Error::DivideZero: return "Sıfıra bölme";
    case Error::Domain: return "Tanımsız işlem";
    case Error::TooLarge: return "İfade/sonuç çok büyük";
    case Error::Memory: return "Bellek yetersiz";
  }
  return "Hata";
}

}  // namespace calctus::math
