#include <cstdio>
#include <cstring>

#include "calctus/hal.h"
#include "services/pi_link.h"

namespace calctus::hal {
bool connected = false;
bool pi_exchange(const uint8_t* tx, size_t size, uint8_t* rx, size_t capacity,
                 size_t& length, uint32_t) {
  length = 0;
  if (!connected) return false;
  pi::Frame in{}, out{};
  if (!pi::decode(tx, size, in)) return false;
  out.sequence = in.sequence;
  out.command = in.command;
  if (in.command == pi::Command::Ping) {
    out.length = 2; out.payload[0] = 'O'; out.payload[1] = 'K';
  } else if (in.command == pi::Command::Info) {
    constexpr char info[] = "Pi mock";
    out.length = sizeof info - 1;
    std::memcpy(out.payload, info, out.length);
  } else if (in.command == pi::Command::Square && in.length == 4) {
    out.length = 4;
    out.payload[0] = 144;
  } else return false;
  length = pi::encode(out, rx, capacity);
  return length != 0;
}
}

int main() {
  calctus::pi::Frame frame{};
  frame.sequence = 3;
  frame.command = calctus::pi::Command::Ping;
  uint8_t bytes[calctus::pi::kMaxFrame]{};
  size_t n = calctus::pi::encode(frame, bytes, sizeof bytes);
  if (n != 8 || !calctus::pi::decode(bytes, n, frame)) return 1;
  bytes[n - 1] ^= 1;
  if (calctus::pi::decode(bytes, n, frame) || calctus::pi::ping()) return 2;
  calctus::hal::connected = true;
  if (!calctus::pi::ping()) return 3;
  char info[49]{};
  if (!calctus::pi::info(info, sizeof info) || std::strcmp(info, "Pi mock")) return 4;
  int32_t result = 0;
  if (!calctus::pi::square(12, result) || result != 144) return 5;
  std::puts("Pi link: CRC rejection, missing Pi, ping, info, remote compute passed");
}
