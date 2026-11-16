#include <cmath>
#include <cstdio>
#include <cstring>

#include "apps/elements/data.h"

int main() {
  using calctus::apps::element_by_number;
  using calctus::apps::element_number_for_symbol;
  using calctus::apps::kElementCount;
  if (kElementCount != 118 || element_by_number(0) || element_by_number(119)) return 1;
  for (int n = 1; n <= kElementCount; ++n) {
    const auto* e = element_by_number(n);
    if (!e || !*e->name || !*e->symbol || e->period < 1 || e->period > 7 ||
        e->group > 18 || e->approximate_mass < n ||
        (e->phase != 'G' && e->phase != 'L' && e->phase != 'S') ||
        (e->block != 's' && e->block != 'p' && e->block != 'd' && e->block != 'f') ||
        element_number_for_symbol(e->symbol) != n) {
      std::fprintf(stderr, "bozuk element: %d\n", n);
      return 2;
    }
  }
  const auto* iron = element_by_number(26);
  if (std::strcmp(iron->name, "Iron") || std::fabs(iron->approximate_mass - 55.845) > 0.01 ||
      iron->period != 4 || iron->group != 8 || element_number_for_symbol("fe") != 26 ||
      element_number_for_symbol("FE") != 26 || element_number_for_symbol("Uue") != 0 ||
      std::strcmp(element_by_number(118)->symbol, "Og") ||
      element_by_number(1)->phase != 'G' || element_by_number(80)->phase != 'L' ||
      element_by_number(57)->block != 'f' || element_by_number(57)->group != 0 ||
      element_number_for_symbol("OgX") != 0 || element_number_for_symbol(nullptr) != 0)
    return 3;
  std::puts("elementler: 118 kayıt, sembol, sınır ve Fe/Og geçti");
  return 0;
}
