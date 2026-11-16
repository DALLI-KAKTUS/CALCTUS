#include "math/sequence.h"

#include <cstdio>
#include <cstring>

namespace calctus::math {

Error generate_sequence(SequenceMode mode, const char* formula, const char* initial,
                        unsigned first, unsigned count, char out[][128], AngleUnit angle) {
  if (!formula || !initial || !out || !*formula || count == 0 || count > 5)
    return Error::Syntax;
  if (first > 100 || count - 1 > 100 - first) return Error::TooLarge;
  if (mode == SequenceMode::Explicit && std::strstr(formula, "Ans"))
    return Error::Variable;

  char rows[5][128]{};
  char previous[128] = "0";
  if (mode == SequenceMode::Recurrence) {
    Error error = evaluate(initial, "0", NumberFormat::Auto,
                           previous, sizeof previous, angle, "0");
    if (error != Error::None) return error;
  }
  for (unsigned n = mode == SequenceMode::Recurrence ? 0 : first;
       n < first + count; ++n) {
    char value[128];
    if (mode == SequenceMode::Recurrence && n == 0) {
      std::strcpy(value, previous);
    } else {
      char index[12];
      std::snprintf(index, sizeof index, "%u", n);
      const Error error = evaluate(formula, previous, NumberFormat::Auto,
                                   value, sizeof value, angle, index);
      if (error != Error::None) return error;
      std::strcpy(previous, value);
    }
    if (n >= first) std::strcpy(rows[n - first], value);
  }
  for (unsigned i = 0; i < count; ++i) std::strcpy(out[i], rows[i]);
  return Error::None;
}

}  // namespace calctus::math
