// Masaüstü simülatörü: HAL'in SDL2 ile gerçeklenmesi ve test araçları.
#pragma once

#include <string>

namespace calctus::sim {

struct Options {
  bool headless = false;    // pencere açmadan, sanal zamanla çalış
  std::string script;       // tuş betiği dosyası
  std::string shot;         // çıkışta ekran görüntüsü (320x240)
  std::string device_shot;  // çıkışta cihaz görüntüsü (kaplama + ekran)
  double scale = 0;         // pencere ölçeği; 0 = ekrana sığan en büyük
};

// Komut satırını okur. false dönerse program çıkmalıdır (exit_code ile).
bool configure(int argc, char** argv, int& exit_code);

// Çıkışta istenen görüntüleri yazar ve pencereyi kapatır.
int finish();

}  // namespace calctus::sim
