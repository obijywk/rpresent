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

 private:
  EGLDisplay display_;
  EGLSurface surface_;
  EGLContext context_;
  VGint width_;
  VGint height_;
};

}  // namespace

#endif  // RPRESENT_WINDOW_H
