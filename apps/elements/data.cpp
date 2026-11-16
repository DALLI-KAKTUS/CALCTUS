// tools/gen_elements.py tarafından üretildi; el ile değiştirmeyin.
// Bilgiler: Bowserinator/Periodic-Table-JSON, a78c7a48 (yalnız olgusal alanlar).
#include "apps/elements/data.h"

#include <cctype>

namespace calctus::apps {
namespace {

constexpr Element kElements[kElementCount] = {
    {"H", "Hydrogen", 1.008f, 1, 1, 'G', 's'},
    {"He", "Helium", 4.0026022f, 1, 18, 'G', 's'},
    {"Li", "Lithium", 6.94f, 2, 1, 'S', 's'},
    {"Be", "Beryllium", 9.01218315f, 2, 2, 'S', 's'},
    {"B", "Boron", 10.81f, 2, 13, 'S', 'p'},
    {"C", "Carbon", 12.011f, 2, 14, 'S', 'p'},
    {"N", "Nitrogen", 14.007f, 2, 15, 'G', 'p'},
    {"O", "Oxygen", 15.999f, 2, 16, 'G', 'p'},
    {"F", "Fluorine", 18.99840316f, 2, 17, 'G', 'p'},
    {"Ne", "Neon", 20.17976f, 2, 18, 'G', 'p'},
    {"Na", "Sodium", 22.98976928f, 3, 1, 'S', 's'},
    {"Mg", "Magnesium", 24.305f, 3, 2, 'S', 's'},
    {"Al", "Aluminium", 26.98153857f, 3, 13, 'S', 'p'},
    {"Si", "Silicon", 28.085f, 3, 14, 'S', 'p'},
    {"P", "Phosphorus", 30.973762f, 3, 15, 'S', 'p'},
    {"S", "Sulfur", 32.06f, 3, 16, 'S', 'p'},
    {"Cl", "Chlorine", 35.45f, 3, 17, 'G', 'p'},
    {"Ar", "Argon", 39.9481f, 3, 18, 'G', 'p'},
    {"K", "Potassium", 39.09831f, 4, 1, 'S', 's'},
    {"Ca", "Calcium", 40.0784f, 4, 2, 'S', 's'},
    {"Sc", "Scandium", 44.9559085f, 4, 3, 'S', 'd'},
    {"Ti", "Titanium", 47.8671f, 4, 4, 'S', 'd'},
    {"V", "Vanadium", 50.94151f, 4, 5, 'S', 'd'},
    {"Cr", "Chromium", 51.99616f, 4, 6, 'S', 'd'},
    {"Mn", "Manganese", 54.9380443f, 4, 7, 'S', 'd'},
    {"Fe", "Iron", 55.8452f, 4, 8, 'S', 'd'},
    {"Co", "Cobalt", 58.9331944f, 4, 9, 'S', 'd'},
    {"Ni", "Nickel", 58.69344f, 4, 10, 'S', 'd'},
    {"Cu", "Copper", 63.5463f, 4, 11, 'S', 'd'},
    {"Zn", "Zinc", 65.382f, 4, 12, 'S', 'd'},
    {"Ga", "Gallium", 69.7231f, 4, 13, 'S', 'p'},
    {"Ge", "Germanium", 72.6308f, 4, 14, 'S', 'p'},
    {"As", "Arsenic", 74.9215956f, 4, 15, 'S', 'p'},
    {"Se", "Selenium", 78.9718f, 4, 16, 'S', 'p'},
    {"Br", "Bromine", 79.904f, 4, 17, 'L', 'p'},
    {"Kr", "Krypton", 83.7982f, 4, 18, 'G', 'p'},
    {"Rb", "Rubidium", 85.46783f, 5, 1, 'S', 's'},
    {"Sr", "Strontium", 87.621f, 5, 2, 'S', 's'},
    {"Y", "Yttrium", 88.905842f, 5, 3, 'S', 'd'},
    {"Zr", "Zirconium", 91.2242f, 5, 4, 'S', 'd'},
    {"Nb", "Niobium", 92.906372f, 5, 5, 'S', 'd'},
    {"Mo", "Molybdenum", 95.951f, 5, 6, 'S', 'd'},
    {"Tc", "Technetium", 98.0f, 5, 7, 'S', 'd'},
    {"Ru", "Ruthenium", 101.072f, 5, 8, 'S', 'd'},
    {"Rh", "Rhodium", 102.905502f, 5, 9, 'S', 'd'},
    {"Pd", "Palladium", 106.421f, 5, 10, 'S', 'd'},
    {"Ag", "Silver", 107.86822f, 5, 11, 'S', 'd'},
    {"Cd", "Cadmium", 112.4144f, 5, 12, 'S', 'd'},
    {"In", "Indium", 114.8181f, 5, 13, 'S', 'p'},
    {"Sn", "Tin", 118.7107f, 5, 14, 'S', 'p'},
    {"Sb", "Antimony", 121.7601f, 5, 15, 'S', 'p'},
    {"Te", "Tellurium", 127.603f, 5, 16, 'S', 'p'},
    {"I", "Iodine", 126.904473f, 5, 17, 'S', 'p'},
    {"Xe", "Xenon", 131.2936f, 5, 18, 'G', 'p'},
    {"Cs", "Cesium", 132.905452f, 6, 1, 'S', 's'},
    {"Ba", "Barium", 137.3277f, 6, 2, 'S', 's'},
    {"La", "Lanthanum", 138.905477f, 6, 0, 'S', 'f'},
    {"Ce", "Cerium", 140.1161f, 6, 0, 'S', 'f'},
    {"Pr", "Praseodymium", 140.907662f, 6, 0, 'S', 'f'},
    {"Nd", "Neodymium", 144.2423f, 6, 0, 'S', 'f'},
    {"Pm", "Promethium", 145.0f, 6, 0, 'S', 'f'},
    {"Sm", "Samarium", 150.362f, 6, 0, 'S', 'f'},
    {"Eu", "Europium", 151.9641f, 6, 0, 'S', 'f'},
    {"Gd", "Gadolinium", 157.253f, 6, 0, 'S', 'f'},
    {"Tb", "Terbium", 158.925352f, 6, 0, 'S', 'f'},
    {"Dy", "Dysprosium", 162.5001f, 6, 0, 'S', 'f'},
    {"Ho", "Holmium", 164.930332f, 6, 0, 'S', 'f'},
    {"Er", "Erbium", 167.2593f, 6, 0, 'S', 'f'},
    {"Tm", "Thulium", 168.934222f, 6, 0, 'S', 'f'},
    {"Yb", "Ytterbium", 173.0451f, 6, 0, 'S', 'f'},
    {"Lu", "Lutetium", 174.96681f, 6, 3, 'S', 'd'},
    {"Hf", "Hafnium", 178.492f, 6, 4, 'S', 'd'},
    {"Ta", "Tantalum", 180.947882f, 6, 5, 'S', 'd'},
    {"W", "Tungsten", 183.841f, 6, 6, 'S', 'd'},
    {"Re", "Rhenium", 186.2071f, 6, 7, 'S', 'd'},
    {"Os", "Osmium", 190.233f, 6, 8, 'S', 'd'},
    {"Ir", "Iridium", 192.2173f, 6, 9, 'S', 'd'},
    {"Pt", "Platinum", 195.0849f, 6, 10, 'S', 'd'},
    {"Au", "Gold", 196.9665695f, 6, 11, 'S', 'd'},
    {"Hg", "Mercury", 200.5923f, 6, 12, 'L', 'd'},
    {"Tl", "Thallium", 204.38f, 6, 13, 'S', 'p'},
    {"Pb", "Lead", 207.21f, 6, 14, 'S', 'p'},
    {"Bi", "Bismuth", 208.980401f, 6, 15, 'S', 'p'},
    {"Po", "Polonium", 209.0f, 6, 16, 'S', 'p'},
    {"At", "Astatine", 210.0f, 6, 17, 'S', 'p'},
    {"Rn", "Radon", 222.0f, 6, 18, 'G', 'p'},
    {"Fr", "Francium", 223.0f, 7, 1, 'S', 's'},
    {"Ra", "Radium", 226.0f, 7, 2, 'S', 's'},
    {"Ac", "Actinium", 227.0f, 7, 0, 'S', 'f'},
    {"Th", "Thorium", 232.03774f, 7, 0, 'S', 'f'},
    {"Pa", "Protactinium", 231.035882f, 7, 0, 'S', 'f'},
    {"U", "Uranium", 238.028913f, 7, 0, 'S', 'f'},
    {"Np", "Neptunium", 237.0f, 7, 0, 'S', 'f'},
    {"Pu", "Plutonium", 244.0f, 7, 0, 'S', 'f'},
    {"Am", "Americium", 243.0f, 7, 0, 'S', 'f'},
    {"Cm", "Curium", 247.0f, 7, 0, 'S', 'f'},
    {"Bk", "Berkelium", 247.0f, 7, 0, 'S', 'f'},
    {"Cf", "Californium", 251.0f, 7, 0, 'S', 'f'},
    {"Es", "Einsteinium", 252.0f, 7, 0, 'S', 'f'},
    {"Fm", "Fermium", 257.0f, 7, 0, 'S', 'f'},
    {"Md", "Mendelevium", 258.0f, 7, 0, 'S', 'f'},
    {"No", "Nobelium", 259.0f, 7, 0, 'S', 'f'},
    {"Lr", "Lawrencium", 266.0f, 7, 3, 'S', 'd'},
    {"Rf", "Rutherfordium", 267.0f, 7, 4, 'S', 'd'},
    {"Db", "Dubnium", 268.0f, 7, 5, 'S', 'd'},
    {"Sg", "Seaborgium", 269.0f, 7, 6, 'S', 'd'},
    {"Bh", "Bohrium", 270.0f, 7, 7, 'S', 'd'},
    {"Hs", "Hassium", 269.0f, 7, 8, 'S', 'd'},
    {"Mt", "Meitnerium", 278.0f, 7, 9, 'S', 'd'},
    {"Ds", "Darmstadtium", 281.0f, 7, 10, 'S', 'd'},
    {"Rg", "Roentgenium", 282.0f, 7, 11, 'S', 'd'},
    {"Cn", "Copernicium", 285.0f, 7, 12, 'L', 'd'},
    {"Nh", "Nihonium", 286.0f, 7, 13, 'S', 'p'},
    {"Fl", "Flerovium", 289.0f, 7, 14, 'S', 'p'},
    {"Mc", "Moscovium", 289.0f, 7, 15, 'S', 'p'},
    {"Lv", "Livermorium", 293.0f, 7, 16, 'S', 'p'},
    {"Ts", "Tennessine", 294.0f, 7, 17, 'S', 'p'},
    {"Og", "Oganesson", 294.0f, 7, 18, 'S', 'p'},
};

}  // namespace

const Element* element_by_number(int number) {
  return number >= 1 && number <= kElementCount ? &kElements[number - 1] : nullptr;
}

int element_number_for_symbol(const char* symbol) {
  if (!symbol || !*symbol) return 0;
  for (int i = 0; i < kElementCount; ++i) {
    const char* name = kElements[i].symbol;
    const char* cursor = symbol;
    while (*name && *cursor &&
           std::tolower(static_cast<unsigned char>(*name)) ==
               std::tolower(static_cast<unsigned char>(*cursor))) {
      ++name;
      ++cursor;
    }
    if (!*name && !*cursor) return i + 1;
  }
  return 0;
}

}  // namespace calctus::apps
