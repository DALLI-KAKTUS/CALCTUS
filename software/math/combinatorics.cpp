#include "math/combinatorics.h"

#include <cstdlib>
#include <cstring>

extern "C" {
#include "libbf.h"
}

namespace calctus::math {
namespace {

void* resize(void*, void* ptr, size_t size) {
  if (!size) { std::free(ptr); return nullptr; }
  return std::realloc(ptr, size);
}

}  // namespace

Error combinatoric(Combinatoric operation, unsigned n, unsigned k,
                   char* out, size_t capacity) {
  if (!out || !capacity) return Error::TooLarge;
  out[0] = '\0';
  if (n > 60) return Error::TooLarge;
  if (operation != Combinatoric::Factorial && k > n) return Error::Domain;

  bf_context_t ctx;
  bf_context_init(&ctx, resize, nullptr);
  bfdec_t value{}, divisor{}, quotient{};
  bfdec_init(&ctx, &value);
  bfdec_init(&ctx, &divisor);
  bfdec_init(&ctx, &quotient);
  int status = bfdec_set_ui(&value, 1);

  if (operation == Combinatoric::Factorial) {
    for (unsigned i = 2; i <= n && !status; ++i)
      status |= bfdec_mul_si(&value, &value, i, BF_PREC_INF, BF_RNDZ);
  } else if (operation == Combinatoric::Permutation) {
    for (unsigned i = n - k + 1; i <= n && !status; ++i)
      status |= bfdec_mul_si(&value, &value, i, BF_PREC_INF, BF_RNDZ);
  } else if (operation == Combinatoric::Combination) {
    if (k > n - k) k = n - k;
    for (unsigned i = 1; i <= k && !status; ++i) {
      status |= bfdec_mul_si(&value, &value, n - k + i, BF_PREC_INF, BF_RNDZ);
      status |= bfdec_set_ui(&divisor, i);
      if (!status) status |= bfdec_div(&quotient, &value, &divisor, 100, BF_RNDN);
      if (!status) status |= bfdec_set(&value, &quotient);
    }
  }

  Error error = status & BF_ST_MEM_ERROR ? Error::Memory :
                status ? Error::Domain : Error::None;
  if (error == Error::None) {
    char* text = bfdec_ftoa(nullptr, &value, 0, BF_FTOA_FORMAT_FRAC | BF_RNDN);
    if (!text) error = Error::Memory;
    else {
      const size_t length = std::strlen(text);
      if (length >= capacity) error = Error::TooLarge;
      else std::memcpy(out, text, length + 1);
      bf_free(&ctx, text);
    }
  }

  bfdec_delete(&quotient);
  bfdec_delete(&divisor);
  bfdec_delete(&value);
  bf_context_end(&ctx);
  return error;
}

}  // namespace calctus::math
