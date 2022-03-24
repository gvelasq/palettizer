#include <cpp11.hpp>
using namespace cpp11;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

[[cpp11::register]]
bool plt_check_(std::string path) {
  int x, y, n, ok;
  ok = stbi_info(path.c_str(), &x, &y, &n);
  return ok;
}
