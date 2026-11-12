#include "apps/keytest/keytest.h"
#include "kernel/app.h"

namespace calctus {

// S0: sistem doğrudan tuş testi uygulamasını açar. Ana ekran S1'de gelecek.
int boot() {
  static apps::KeyTestApp app;
  return run(app);
}

}  // namespace calctus
