#include <cpp11.hpp>

#include "stb_image/stb_image.h"

using namespace cpp11;

[[cpp11::register]]
bool plt_check_(std::string path) {
  int x, y, n, ok;
  ok = stbi_info(path.c_str(), &x, &y, &n);
  return ok;
}
