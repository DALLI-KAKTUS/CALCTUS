#include "services/micropython/runtime.h"

#include <cstdlib>
#include <cstring>

#include "calctus/hal.h"

extern "C" {
#include "port/micropython_embed.h"
#include "py/stackctrl.h"
int calctus_python_exec(const char* source);
}

namespace calctus::python {
namespace {
constexpr size_t kHeapSize = 96 * 1024;
Result* output = nullptr;
unsigned long loop_count = 0;
}  // namespace

void run(const char* source, Result& result) {
  result = {};
  if (!source || std::strlen(source) > 4095) {
    std::strcpy(result.text, "Kaynak 4095 baytı aşamaz");
    result.length = std::strlen(result.text);
    return;
  }
  void* heap = std::malloc(kHeapSize);
  if (!heap) {
    std::strcpy(result.text, "MicroPython belleği ayrılamadı");
    result.length = std::strlen(result.text);
    return;
  }
  output = &result;
  loop_count = 0;
  int stack_top;
  mp_embed_init(heap, kHeapSize, &stack_top);
  mp_stack_set_limit(8 * 1024);
  result.ok = calctus_python_exec(source) != 0;
  mp_embed_deinit();
  output = nullptr;
  std::free(heap);
}

}  // namespace calctus::python

extern "C" void calctus_python_output(const char* str, size_t length) {
  using namespace calctus::python;
  if (!output || !str) return;
  const size_t free = sizeof(output->text) - 1 - output->length;
  const size_t copied = length < free ? length : free;
  std::memcpy(output->text + output->length, str, copied);
  output->length += copied;
  output->text[output->length] = '\0';
  if (copied < length) output->truncated = true;
}

extern "C" int calctus_python_limit_reached(void) {
  return ++calctus::python::loop_count > 100000;
}

extern "C" int calctus_python_import_read(const char* name, char* data,
                                            size_t capacity, size_t* length) {
  if (!data || !length || capacity < 2) return 0;
  *length = 0;
  if (!calctus::hal::file_read(name, data, capacity, *length)) return 0;
  for (size_t i = 0; i < *length; ++i)
    if (data[i] != '\n' && data[i] != '\r' && data[i] != '\t' &&
        (data[i] < ' ' || data[i] > '~')) return 0;
  return 1;
}

extern "C" int calctus_python_file_read(const char* name, char* data,
                                          size_t capacity, size_t* length) {
  if (!data || !length || capacity < 2) return 0;
  *length = 0;
  if (!calctus::hal::file_read(name, data, capacity, *length)) return 0;
  for (size_t i = 0; i < *length; ++i)
    if (data[i] != '\n' && data[i] != '\r' && data[i] != '\t' &&
        (data[i] < ' ' || data[i] > '~')) return 0;
  return 1;
}

extern "C" int calctus_python_file_write(const char* name, const char* data,
                                           size_t length) {
  if (!name || !data || length > 4095) return 0;
  for (size_t i = 0; i < length; ++i)
    if (data[i] != '\n' && data[i] != '\r' && data[i] != '\t' &&
        (data[i] < ' ' || data[i] > '~')) return 0;
  return calctus::hal::file_write(name, data, length);
}
