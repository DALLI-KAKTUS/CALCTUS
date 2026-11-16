#pragma once

#include <cstddef>

#include "calctus/hal.h"

namespace calctus::files {

constexpr int kMaxEntries = 32;
constexpr size_t kNameCapacity = sizeof(hal::FileInfo::name);
constexpr size_t kTextCapacity = 4096; // Editör sınırı; SD kartın dosya sınırı değil.

struct Document {
  char name[kNameCapacity]{};
  char text[kTextCapacity]{};
  size_t length = 0;
};

bool valid_name(const char* name);
// Kart yoksa -1; diğer durumda doldurulan kayıt sayısı (en çok capacity).
int list(hal::FileInfo* out, int capacity);
bool load(const char* name, Document& out);
bool save(const Document& doc);
bool rename(const char* from, const char* to);
bool erase(const char* name);

}  // namespace calctus::files
