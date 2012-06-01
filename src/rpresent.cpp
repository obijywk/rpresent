#include <stdlib.h>
#include <sys/time.h>
#include <VG/openvg.h>
#include "text.h"
#include "window.h"

int main(int argc, char** argv) {
  rpresent::Window window;
  window.Initialize();

  rpresent::Text text;
  text.Initialize();

  VGfloat red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
  VGfloat green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
  VGfloat blue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
  VGfloat black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

  VGPaint paint = vgCreatePaint();
  vgSetPaint(paint, VG_FILL_PATH | VG_STROKE_PATH);

  timespec last_clock, clock;
  clock_gettime(CLOCK_REALTIME, &last_clock);
  int fps = 0;
  char timestr[16];
  strcpy(timestr, "0");
  while (true) {
    window.Clear();
    for (int i = 0; i < 16; i++) {
      vgSetParameterfv(paint, VG_PAINT_COLOR, 4, red);
      text.DrawString(200, 32 + i * 64, "Hello World");
      vgSetParameterfv(paint, VG_PAINT_COLOR, 4, green);
      text.DrawString(800, 32 + i * 64, "Hello World");
      vgSetParameterfv(paint, VG_PAINT_COLOR, 4, blue);
      text.DrawString(1400, 32 + i * 64, "Hello World");
    }
    clock_gettime(CLOCK_REALTIME, &clock);
    if (clock.tv_sec > last_clock.tv_sec) {
      sprintf(timestr, "%d", fps);
      fps = 0;
      last_clock = clock;
    }
    vgSetParameterfv(paint, VG_PAINT_COLOR, 4, black);
    text.DrawString(64, 32, timestr);
    window.SwapBuffers();
    ++fps;
  }

  return 0;
}
