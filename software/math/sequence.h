#pragma once

#include <cstdint>

#include "math/eval.h"

namespace calctus::math {

enum class SequenceMode : uint8_t { Explicit, Recurrence };

// Açık: u(n)=f(x=n). Yineleme: u(0)=initial, u(n)=f(x=n, Ans=u(n-1)).
// En çok beş satır, son n <= 100; başarılıysa out'a yazılır.
Error generate_sequence(SequenceMode mode, const char* formula, const char* initial,
                        unsigned first, unsigned count, char out[][128],
                        AngleUnit angle = AngleUnit::Degree);

}  // namespace calctus::math
