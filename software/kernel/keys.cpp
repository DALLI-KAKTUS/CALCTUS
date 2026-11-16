#include <cstring>

#include "calctus/keys.h"

namespace calctus {

namespace {

const KeyInfo kKeys[] = {
#define CALCTUS_KEY(id, name, label, kind, x, y, w, h) {name, label, KeyKind::kind, x, y, w, h},
#include "calctus/keymap.def"
#undef CALCTUS_KEY
};

const KeyInfo kNoKey = {"NONE", "", KeyKind::Nav, 0, 0, 0, 0};

}  // namespace

const KeyInfo& key_info(Key key) {
  const int i = key_index(key);
  if (i < 0 || i >= kKeyCount) return kNoKey;
  return kKeys[i];
}

Key key_from_name(const char* name) {
  for (int i = 0; i < kKeyCount; ++i) {
    if (std::strcmp(kKeys[i].name, name) == 0) return static_cast<Key>(i);
  }
  return Key::None;
}

}  // namespace calctus
