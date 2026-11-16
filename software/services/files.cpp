#include "services/files.h"

#include <cstring>

namespace calctus::files {

bool valid_name(const char* name) {
  if (!name || !*name) return false;
  size_t count = 0;
  for (; count < kNameCapacity && name[count]; ++count) {
    const char c = name[count];
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.')) return false;
  }
  return count > 0 && count < kNameCapacity &&
         std::strcmp(name, ".") != 0 && std::strcmp(name, "..") != 0;
}

int list(hal::FileInfo* out, int capacity) {
  if (!out || capacity < 0) return -1;
  const int count = hal::files_list(out, capacity);
  if (count < 0) return count;
  // Aynı dosya sırası iki platformda da görünür; en fazla 32 giriş sıralanır.
  for (int i = 1; i < count; ++i) {
    hal::FileInfo current = out[i];
    int j = i;
    while (j > 0 && std::strcmp(out[j - 1].name, current.name) > 0) {
      out[j] = out[j - 1]; --j;
    }
    out[j] = current;
  }
  return count;
}

bool load(const char* name, Document& out) {
  out = {};
  if (!valid_name(name)) return false;
  size_t length = 0;
  if (!hal::file_read(name, out.text, sizeof out.text, length) || length >= kTextCapacity)
    return false;
  for (size_t i = 0; i < length; ++i) {
    const char c = out.text[i];
    if (c != '\n' && (c < ' ' || c > '~')) { out = {}; return false; }
  }
  std::memcpy(out.name, name, std::strlen(name) + 1);
  out.length = length;
  out.text[length] = '\0';
  return true;
}

bool save(const Document& doc) {
  if (!valid_name(doc.name) || doc.length >= kTextCapacity ||
      doc.text[doc.length] != '\0') return false;
  for (size_t i = 0; i < doc.length; ++i)
    if (doc.text[i] != '\n' && (doc.text[i] < ' ' || doc.text[i] > '~')) return false;
  return hal::file_write(doc.name, doc.text, doc.length);
}

bool erase(const char* name) {
  return valid_name(name) && hal::file_delete(name);
}

bool rename(const char* from, const char* to) {
  return valid_name(from) && valid_name(to) && hal::file_rename(from, to);
}

}  // namespace calctus::files
