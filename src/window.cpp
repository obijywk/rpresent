#include <stdio.h>
#include "window.h"

namespace rpresent {

Window::Window()
    :
#ifdef PLATFORM_X11
      x_display_(NULL),
#endif
#ifdef PLATFORM_PI
      dispman_display_(0),
      dispman_update_(0),
      dispman_element_(0),
#endif
      display_(EGL_NO_DISPLAY),
      surface_(EGL_NO_SURFACE),
      context_(EGL_NO_CONTEXT),
      width_(1920),
      height_(1080) {
}

Window::~Window() {
  if (display_ != EGL_NO_DISPLAY) {
    if (!eglMakeCurrent(
          display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
      fprintf(stderr, "eglMakeCurrent failed\n");
    }
    if (!eglTerminate(display_)) {
      fprintf(stderr, "eglTerminate failed\n");
    }
  }

#ifdef PLATFORM_X11
  if (x_display_) {
    XCloseDisplay(x_display_);
  }
#endif

#ifdef PLATFORM_PI
  if (dispman_update_ && dispman_element_) {
    vc_dispmanx_element_remove(dispman_update_, dispman_element_);
  }
  if (dispman_display_) {
    vc_dispmanx_display_close(dispman_display_);
  }
#endif
}

bool Window::Initialize() {
  EGLNativeDisplayType native_display;
  EGLNativeWindowType native_window;

#ifdef PLATFORM_X11
  // TODO: choose a reasonable window size

  x_display_ = XOpenDisplay(0);
  if (!x_display_) {
    fprintf(stderr, "XOpenDisplay failed\n");
    return false;
  }

  long x_screen = XDefaultScreen(x_display_);
  ::Window root_window = RootWindow(x_display_, x_screen);
  int depth = DefaultDepth(x_display_, x_screen);
  XVisualInfo x_visual_info;
  XMatchVisualInfo(x_display_, x_screen, depth, TrueColor, &x_visual_info);

  XSetWindowAttributes set_window_attrs;
  set_window_attrs.colormap = XCreateColormap(x_display_, root_window,
					      x_visual_info.visual, AllocNone);
  set_window_attrs.event_mask =
      StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask;

  ::Window x_window = XCreateWindow(x_display_, root_window,
                                    0, 0, width_, height_,
                                    0, CopyFromParent,
                                    InputOutput, CopyFromParent,
                                    CWBackPixel | CWBorderPixel |
                                    CWEventMask | CWColormap,
                                    &set_window_attrs);
  XMapWindow(x_display_, x_window);
  XFlush(x_display_);

  native_display = static_cast<EGLNativeDisplayType>(x_display_);
  native_window = static_cast<EGLNativeWindowType>(x_window);
#endif

#ifdef PLATFORM_PI
  bcm_host_init();

  uint32_t bcm_width, bcm_height;
  int32_t success = graphics_get_display_size(0, &bcm_width, &bcm_height);
  if (success < 0) {
    fprintf(stderr, "graphics_get_display_size failed\n");
    return false;
  }
  width_ = bcm_width;
  height_ = bcm_height;

  dispman_display_ = vc_dispmanx_display_open(0);
  dispman_update_ = vc_dispmanx_update_start(0);

  VC_RECT_T dst_rect, src_rect;
  dst_rect.x = 0;
  dst_rect.y = 0;
  dst_rect.width = bcm_width;
  dst_rect.height = bcm_height;
  src_rect.x = 0;
  src_rect.y = 0;
  src_rect.width = bcm_width << 16;
  src_rect.height = bcm_height << 16;
  dispman_element_ =
      vc_dispmanx_element_add(dispman_update_, dispman_display_,
                              0,  // layer
                              &dst_rect,
                              0,  // src
                              &src_rect,
                              DISPMANX_PROTECTION_NONE,
                              0,  // alpha
                              0,  // clamp
                              DISPMANX_NO_ROTATE  // transform
                              );

  vc_dispmanx_update_submit_sync(dispman_update_);

  egl_window_.element = dispman_element_;
  egl_window_.width = bcm_width;
  egl_window_.height = bcm_height;

  native_display = EGL_DEFAULT_DISPLAY;
  native_window = static_cast<EGLNativeWindowType>(&egl_window_);
#endif

  display_ = eglGetDisplay(native_display);
  if (display_ == EGL_NO_DISPLAY) {
    fprintf(stderr, "eglGetDisplay failed\n");
    return false;
  }

  EGLint major_version, minor_version;
  if (!eglInitialize(display_, &major_version, &minor_version)) {
    fprintf(stderr, "eglInitialize failed\n");
    return false;
  }

  eglBindAPI(EGL_OPENVG_API);
  if (eglGetError() != EGL_SUCCESS) {
    fprintf(stderr, "eglBindAPI failed\n");
    return false;
  }

  EGLint config_attribs[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    // TODO: why doesn't alpha work on X11?
    EGL_ALPHA_SIZE, 0,
    EGL_NONE,
  };
  EGLConfig config = 0;
  int num_configs = 0;
  if (!eglChooseConfig(display_, config_attribs, &config, 1, &num_configs)) {
    fprintf(stderr, "eglChooseConfig failed\n");
    return false;
  }
  if (num_configs != 1) {
    fprintf(stderr, "eglChooseConfig returned no configs\n");
    return false;
  }

  EGLint context_attribs[] = {
#ifdef PLATFORM_PI
    EGL_CONTEXT_CLIENT_VERSION, 1,
#endif
    EGL_NONE,
  };
  context_ = eglCreateContext(display_, config,
			      EGL_NO_CONTEXT, context_attribs);
  EGLint error = eglGetError();
  if (error != EGL_SUCCESS) {
    fprintf(stderr, "eglCreateContext failed %d\n", error);
    return false;
  }

  surface_ = eglCreateWindowSurface(display_, config, native_window, NULL);
  if (eglGetError() != EGL_SUCCESS) {
    fprintf(stderr, "eglCreateWindowSurface failed\n");
    return false;
  }

  eglMakeCurrent(display_, surface_, surface_, context_);
  if (eglGetError() != EGL_SUCCESS) {
    fprintf(stderr, "eglMakeCurrent failed\n");
    return false;
  }

  vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
  vgSeti(VG_MASKING, VG_TRUE);

  VGfloat afClearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  vgSetfv(VG_CLEAR_COLOR, 4, afClearColor);

  vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_BETTER);
  vgLoadIdentity();
  vgScale(static_cast<VGfloat>(width_), static_cast<VGfloat>(height_));

  return true;
}

void Window::SwapBuffers() {
  eglSwapBuffers(display_, surface_);
}

void Window::Clear() {
  vgClear(0, 0, width_, height_);
}

bool Window::HandleEvents() {
  bool run = true;

#ifdef PLATFORM_X11
  XEvent event;
  KeySym keysym;
  while (XPending(x_display_)) {
    XNextEvent(x_display_, &event);
    switch (event.type) {
      case KeyPress:
        keysym = XLookupKeysym((XKeyEvent*)(&event), 0);
        if (keysym == XK_q) {
          run = false;
        }
        break;
      default:
        break;
    }
  }
#endif

#ifdef PLATFORM_PI
  // TODO: pi input
#endif

  return run;
}

}  // namespace rpresent
