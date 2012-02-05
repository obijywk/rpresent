#include <unistd.h>
#include "x11_window.h"

using namespace rpresent;

int main(int argc, char** argv) {
  X11Window window;
  while (window.ProcessEvents()) {
    usleep(10000);
  }
  return 0;
}
