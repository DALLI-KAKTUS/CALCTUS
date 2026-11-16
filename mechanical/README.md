# Kasa ve 3B modeller

CALCTUS'un kasası, tuş başlıkları ve iç parçaları. Henüz model yok; aşağıdaki
düzen modellemeye başlandığında kullanılacak.

```
cad/        düzenlenebilir kaynak modeller (FreeCAD .FCStd, OpenSCAD .scad vb.)
stl/        3B baskıya hazır çıktılar
step/       başka CAD programlarına aktarım için STEP çıktıları (PCB modeliyle eşleşme dahil)
drawings/   ölçülü teknik çizimler (PDF/SVG) ve görseller
```

Kural: her STL/STEP dosyasının `cad/` altında düzenlenebilir bir kaynağı
olmalı; yalnız çıktı olarak gelen model eklenmez.

## Hedefler

- Boyut yaklaşık 165 × 86 × 14 mm
- Ekran penceresi GMT020'ye göre (2,0", 320×240 yatay)
- 55 tuşluk düzen; ilk sürümde 3B baskı tuş başlıkları
- Üst kenarda modül yuvası açıklığı, yanda Reset/Boot erişimi, USB-C
