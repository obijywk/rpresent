#include <stdio.h>
#include "weather.h"
#include "window.h"

int main(int argc, char** argv) {
  rpresent::Window window;
  if (!window.Initialize()) {
    return 1;
  }

  rpresent::Weather weather;
  if (!weather.Initialize("New+York")) {
    return 1;
  }

  while (window.HandleEvents()) {
    window.Clear();
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
    vgLoadIdentity();
    vgTranslate(100, 100);
    vgDrawImage(weather.Icon());
    window.SwapBuffers();
  }

  return 0;
}
