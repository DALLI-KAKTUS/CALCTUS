#include "math/solver.h"

#include <cstring>

namespace calctus::math {
namespace {

Error calc(const char* expression, const char* ans, const char* x, char* out) {
  return evaluate(expression, ans, NumberFormat::Auto, out, 128, AngleUnit::Degree, x);
}

// Üstel gösterim dahil tüm rakamları denetler; -0 ve 0.0 da sıfırdır.
bool zero(const char* text) {
  for (; *text && *text != 'e' && *text != 'E'; ++text)
    if (*text >= '1' && *text <= '9') return false;
  return true;
}

}  // namespace

Error solve_quadratic(const char* a, const char* b, const char* c, Solution& out) {
  if (!a || !b || !c) return Error::Syntax;
  char av[128], bv[128], cv[128];
  Error e = calc("Ans", a, "0", av);
  if (e != Error::None) return e;
  e = calc("Ans", b, "0", bv);
  if (e != Error::None) return e;
  e = calc("Ans", c, "0", cv);
  if (e != Error::None) return e;

  Solution next{};
  if (zero(av)) {
    if (zero(bv)) next.kind = zero(cv) ? Roots::Any : Roots::None;
    else {
      next.kind = Roots::Linear;
      e = calc("-x/Ans", bv, cv, next.first);
      if (e != Error::None) return e;
    }
    if (*next.first && zero(next.first)) std::strcpy(next.first, "0");
    out = next;
    return Error::None;
  }

  char b_square[128], ac[128], four_ac[128], discriminant[128], denominator[128];
  e = calc("Ans^2", bv, "0", b_square);
  if (e != Error::None) return e;
  e = calc("x*Ans", cv, av, ac);
  if (e != Error::None) return e;
  e = calc("4*Ans", ac, "0", four_ac);
  if (e != Error::None) return e;
  e = calc("x-Ans", four_ac, b_square, discriminant);
  if (e != Error::None) return e;
  e = calc("2*Ans", av, "0", denominator);
  if (e != Error::None) return e;

  if (discriminant[0] == '-' && !zero(discriminant)) {
    next.kind = Roots::Complex;
    e = calc("-x/Ans", denominator, bv, next.first);
    if (e != Error::None) return e;
    char magnitude[128], imaginary[128];
    e = calc("-Ans", discriminant, "0", magnitude);
    if (e != Error::None) return e;
    e = calc("sqrt(Ans)", magnitude, "0", imaginary);
    if (e != Error::None) return e;
    e = calc("x/Ans", denominator, imaginary, next.second);
    if (e != Error::None) return e;
    if (next.second[0] == '-') {
      char positive[128];
      e = calc("-Ans", next.second, "0", positive);
      if (e != Error::None) return e;
      std::strcpy(next.second, positive);
    }
  } else if (zero(discriminant)) {
    next.kind = Roots::Double;
    e = calc("-x/Ans", denominator, bv, next.first);
    if (e != Error::None) return e;
  } else {
    next.kind = Roots::Two;
    char root_d[128], q[128], sum[128];
    e = calc("sqrt(Ans)", discriminant, "0", root_d);
    if (e != Error::None) return e;
    // Kararlı formül: q = -(b + sign(b)*sqrt(D))/2; kökler q/a ve c/q.
    e = calc(bv[0] == '-' ? "x-Ans" : "x+Ans", root_d, bv, sum);
    if (e != Error::None) return e;
    e = calc("-Ans/2", sum, "0", q);
    if (e != Error::None) return e;
    e = calc("x/Ans", av, q, next.first);
    if (e != Error::None) return e;
    e = calc("x/Ans", q, cv, next.second);
    if (e != Error::None) return e;
  }
  if (*next.first && zero(next.first)) std::strcpy(next.first, "0");
  if (*next.second && zero(next.second)) std::strcpy(next.second, "0");
  out = next;
  return Error::None;
}

}  // namespace calctus::math
