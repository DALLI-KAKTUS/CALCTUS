#include <cstdio>
#include <cstring>

#include "calctus/hal.h"
#include "services/micropython/runtime.h"

namespace calctus::hal {
char mock_file[4096]{};
size_t mock_length = 0;
bool mock_exists = false;
bool file_read(const char* name, char* data, size_t capacity, size_t& length) {
  constexpr char source[] = "def f(n):\n    return n * n\n";
  length = 0;
  if (!std::strcmp(name, "demo.py")) {
    if (sizeof source > capacity) return false;
    std::memcpy(data, source, sizeof source);
    length = sizeof source - 1;
    return true;
  }
  if (std::strcmp(name, "data.txt") || !mock_exists || mock_length >= capacity) return false;
  std::memcpy(data, mock_file, mock_length);
  data[mock_length] = '\0';
  length = mock_length;
  return true;
}
bool file_write(const char* name, const char* data, size_t length) {
  if (std::strcmp(name, "data.txt") || length >= sizeof mock_file) return false;
  std::memcpy(mock_file, data, length);
  mock_file[length] = '\0';
  mock_length = length;
  mock_exists = true;
  return true;
}
}  // namespace calctus::hal

int main() {
  calctus::python::Result output{};
  calctus::python::run(
      "def triangular(n):\n"
      "    result = 0\n"
      "    for i in range(n + 1):\n"
      "        result += i\n"
      "    return result\n"
      "print(triangular(5))\n", output);
  if (!output.ok || std::strcmp(output.text, "15\n")) {
    std::fprintf(stderr, "Python fonksiyon/döngü: %s\n", output.text);
    return 1;
  }
  calctus::python::run("a = [x * x for x in range(4)]\nprint(a)\n", output);
  if (!output.ok || std::strcmp(output.text, "[0, 1, 4, 9]\n")) return 2;
  calctus::python::run("import math\nprint(math.sqrt(9))\n", output);
  if (!output.ok || std::strncmp(output.text, "3.0\n", 4)) return 3;
  calctus::python::run("import demo\nprint(demo.f(7))\n", output);
  if (!output.ok || std::strcmp(output.text, "49\n")) {
    std::fprintf(stderr, "Python SD import: %s\n", output.text);
    return 7;
  }
  calctus::python::run("print(open('demo.py').read())\n", output);
  if (!output.ok || std::strcmp(output.text, "def f(n):\n    return n * n\n\n")) return 8;
  calctus::python::run("import missing\n", output);
  if (output.ok || !std::strstr(output.text, "ImportError")) return 9;
  calctus::python::run(
      "with open('data.txt', 'w') as f:\n"
      "    f.write('hello\\nworld')\n"
      "with open('data.txt') as f:\n"
      "    print(f.readline().strip())\n"
      "    print(f.read())\n", output);
  if (!output.ok || std::strcmp(output.text, "11\nhello\nworld\n") ||
      std::strcmp(calctus::hal::mock_file, "hello\nworld")) {
    std::fprintf(stderr, "Python open SD: %s\n", output.text);
    return 10;
  }
  calctus::python::run(
      "with open('data.txt', 'a') as f:\n    f.write('!')\n"
      "print(open('data.txt').read())\n", output);
  if (!output.ok || std::strcmp(output.text, "1\nhello\nworld!\n")) {
    std::fprintf(stderr, "Python append SD: %s\n", output.text);
    return 11;
  }
  calctus::python::run("raise ValueError('oops')\n", output);
  if (output.ok || !std::strstr(output.text, "ValueError: oops")) return 4;
  calctus::python::run("while True:\n    pass\n", output);
  if (output.ok || !std::strstr(output.text, "execution limit")) return 5;
  calctus::python::run("print(a)\n", output);
  if (output.ok || !std::strstr(output.text, "NameError")) return 6;
  std::puts("MicroPython: fonksiyon, döngü, liste, math, SD import, hata ve sınır geçti");
  return 0;
}
