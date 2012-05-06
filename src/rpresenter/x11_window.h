#ifndef RPRESENTER_X11_WINDOW_H
#define RPRESENTER_X11_WINDOW_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>

namespace rpresent {

class X11Window {
 public:
  X11Window();
  ~X11Window();

  void SwapBuffers();

  // returns false if the window was closed
  bool ProcessEvents();

 private:
  Display* display_;
  Window window_;
  GLXContext glx_context_;
};

}

#endif
