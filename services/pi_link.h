#pragma once

#include <cstddef>
#include <cstdint>

namespace calctus::pi {

// UART: A5 5A, version=1, sequence, command, length, payload, CRC16-CCITT
// (CRC version..payload, initial FFFF, polynomial 1021, little-endian CRC).
// All messages are bounded; no dynamic allocation or OS calls in common code.
constexpr uint8_t kVersion = 1;
constexpr size_t kMaxPayload = 48;
constexpr size_t kMaxFrame = kMaxPayload + 8;
enum class Command : uint8_t { Ping = 1, Info = 2, Square = 3, Error = 127 };
struct Frame {
  uint8_t sequence = 0;
  Command command = Command::Error;
  uint8_t length = 0;
  uint8_t payload[kMaxPayload]{};
};

size_t encode(const Frame& frame, uint8_t* out, size_t capacity);
bool decode(const uint8_t* data, size_t length, Frame& out);

// One bounded request/response. Missing/malformed Pi returns false.
bool request(Command command, const uint8_t* data, size_t length, Frame& response);
bool ping();
bool info(char* out, size_t capacity);
bool square(int32_t number, int32_t& result);

}  // namespace calctus::pi
