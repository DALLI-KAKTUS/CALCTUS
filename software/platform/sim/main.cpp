#include "kernel/app.h"
#include "platform/sim/sim.h"

int main(int argc, char** argv) {
  int exit_code = 0;
  if (!calctus::sim::configure(argc, argv, exit_code)) return exit_code;
  calctus::boot();
  return calctus::sim::finish();
}
