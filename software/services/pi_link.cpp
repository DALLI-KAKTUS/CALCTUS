#include "services/pi_link.h"

#include <cstring>

#include "calctus/hal.h"

namespace calctus::pi {
namespace {
uint8_t sequence = 0;

uint16_t crc(const uint8_t* bytes, size_t length) {
  uint16_t value = 0xffff;
  for (size_t i = 0; i < length; ++i) {
    value ^= static_cast<uint16_t>(bytes[i]) << 8;
    for (int bit = 0; bit < 8; ++bit)
      value = (value & 0x8000) ? static_cast<uint16_t>((value << 1) ^ 0x1021)
                               : static_cast<uint16_t>(value << 1);
  }
  return value;
}
}  // namespace

size_t encode(const Frame& frame, uint8_t* out, size_t capacity) {
  if (!out || frame.length > kMaxPayload || capacity < frame.length + 8) return 0;
  out[0] = 0xa5; out[1] = 0x5a; out[2] = kVersion;
  out[3] = frame.sequence; out[4] = static_cast<uint8_t>(frame.command);
  out[5] = frame.length;
  if (frame.length) std::memcpy(out + 6, frame.payload, frame.length);
  const uint16_t checksum = crc(out + 2, frame.length + 4);
  out[6 + frame.length] = checksum & 0xff;
  out[7 + frame.length] = checksum >> 8;
  return frame.length + 8;
}

bool decode(const uint8_t* data, size_t length, Frame& out) {
  if (!data || length < 8 || data[0] != 0xa5 || data[1] != 0x5a ||
      data[2] != kVersion || data[5] > kMaxPayload || length != data[5] + 8)
    return false;
  const uint16_t checksum = crc(data + 2, data[5] + 4);
  if (checksum != (static_cast<uint16_t>(data[length - 1]) << 8 | data[length - 2]))
    return false;
  Frame value{};
  value.sequence = data[3];
  value.command = static_cast<Command>(data[4]);
  value.length = data[5];
  if (value.length) std::memcpy(value.payload, data + 6, value.length);
  out = value;
  return true;
}

bool request(Command command, const uint8_t* data, size_t length, Frame& response) {
  if (length > kMaxPayload || (length && !data)) return false;
  Frame message{};
  message.sequence = ++sequence;
  message.command = command;
  message.length = static_cast<uint8_t>(length);
  if (length) std::memcpy(message.payload, data, length);
  uint8_t tx[kMaxFrame]{}, rx[kMaxFrame]{};
  const size_t size = encode(message, tx, sizeof tx);
  size_t received = 0;
  if (!size || !hal::pi_exchange(tx, size, rx, sizeof rx, received, 400) ||
      !decode(rx, received, response)) return false;
  return response.sequence == message.sequence && response.command == command;
}

bool ping() {
  Frame response{};
  return request(Command::Ping, nullptr, 0, response) &&
         response.length == 2 && response.payload[0] == 'O' && response.payload[1] == 'K';
}

bool info(char* out, size_t capacity) {
  Frame response{};
  if (!out || !capacity || !request(Command::Info, nullptr, 0, response) ||
      response.length >= capacity) return false;
  std::memcpy(out, response.payload, response.length);
  out[response.length] = '\0';
  return true;
}

bool square(int32_t number, int32_t& result) {
  uint8_t request_data[4];
  const uint32_t input = static_cast<uint32_t>(number);
  for (int i = 0; i < 4; ++i) request_data[i] = (input >> (i * 8)) & 0xff;
  Frame response{};
  if (!request(Command::Square, request_data, sizeof request_data, response) ||
      response.length != 4) return false;
  uint32_t value = 0;
  for (int i = 0; i < 4; ++i) value |= static_cast<uint32_t>(response.payload[i]) << (i * 8);
  result = static_cast<int32_t>(value);
  return true;
}

}  // namespace calctus::pi
