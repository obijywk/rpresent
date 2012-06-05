#ifndef RPRESENT_WINDOW_H
#define RPRESENT_WINDOW_H

#include <EGL/egl.h>
#include <VG/openvg.h>

#ifdef PLATFORM_PI
#include <bcm_host.h>
#endif

namespace rpresent {

class Window {
 public:
  Window();
  ~Window();

  bool Initialize();
  void Clear();
  void SwapBuffers();

  // Returns false on quit.
  // TODO: return some info about events
  bool HandleEvents();

 private:

#ifdef PLATFORM_X11
  Display* x_display_;
#endif

#ifdef PLATFORM_PI
  DISPMANX_DISPLAY_HANDLE_T dispman_display_;
  DISPMANX_UPDATE_HANDLE_T dispman_update_;
  DISPMANX_ELEMENT_HANDLE_T dispman_element_;
  EGL_DISPMANX_WINDOW_T egl_window_;
#endif

  EGLDisplay display_;
  EGLSurface surface_;
  EGLContext context_;

  VGint width_;
  VGint height_;
};

}  // namespace

#endif  // RPRESENT_WINDOW_H
