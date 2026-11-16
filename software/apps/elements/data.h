#pragma once

#include <cstdint>

namespace calctus::apps {

struct Element {
  const char* symbol;
  const char* name;  // uluslararası ad; atom sembolü evrenseldir
  float approximate_mass; // u; sentetik elementlerde izotop kütle numarası yaklaşık olabilir
  uint8_t period;
  uint8_t group;  // f bloğu için 0: geleneksel grup atanmaz
  char phase;     // G: gaz, L: sıvı, S: katı (sentetikler için tahmini)
  char block;     // s, p, d, f
};

constexpr int kElementCount = 118;
const Element* element_by_number(int atomic_number);
int element_number_for_symbol(const char* symbol);

}  // namespace calctus::apps
