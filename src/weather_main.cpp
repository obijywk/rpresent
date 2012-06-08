#include <stdio.h>
#include "text.h"
#include "weather.h"
#include "window.h"

int main(int argc, char** argv) {
  rpresent::Window window;
  if (!window.Initialize()) {
    return 1;
  }

  rpresent::Text text;
  if (!text.Initialize()) {
    return 1;
  }

  rpresent::Weather weather;
  if (!weather.Initialize("New+York")) {
    return 1;
  }

  VGPaint paint = vgCreatePaint();
  vgSetPaint(paint, VG_FILL_PATH | VG_STROKE_PATH);
  VGfloat black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  vgSetParameterfv(paint, VG_PAINT_COLOR, 4, black);

  int y_offset = 0;
  int y_delta = 1;

  while (window.HandleEvents()) {
    window.Clear();

    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    text.DrawString(600, 900 - y_offset, weather.City());
    text.DrawString(600, 820 - y_offset, weather.Condition());
    text.DrawString(600, 740 - y_offset, weather.TempF());

    vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
    vgLoadIdentity();
    vgTranslate(100, 650 - y_offset);
    vgScale(10.0, 10.0);
    vgDrawImage(weather.Icon());

    window.SwapBuffers();

    y_offset += y_delta;
    if (y_offset <= 0 || y_offset >= 400) {
      y_delta = -y_delta;
    }
  }

  return 0;
}
