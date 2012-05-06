// #define PLATFORM_X11
// #define PLATFORM_PI

#include <stdio.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#ifdef PLATFORM_PI
#include <bcm_host.h>
#endif

bool create_window(EGLNativeDisplayType* native_display,
		   EGLNativeWindowType* native_window) {
#ifdef PLATFORM_X11
  Display* x_display = XOpenDisplay(0);
  if (!x_display) {
    fprintf(stderr, "XOpenDisplay failed\n");
    return false;
  }

  long x_screen = XDefaultScreen(x_display);
  Window root_window = RootWindow(x_display, x_screen);
  int depth = DefaultDepth(x_display, x_screen);
  XVisualInfo x_visual_info;
  XMatchVisualInfo(x_display, x_screen, depth, TrueColor, &x_visual_info);

  XSetWindowAttributes set_window_attrs;
  set_window_attrs.colormap = XCreateColormap(x_display, root_window,
					      x_visual_info.visual, AllocNone);
  set_window_attrs.event_mask = StructureNotifyMask | ExposureMask |
    ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask;

  // TODO: make window full screen
  Window x_window = XCreateWindow(x_display, root_window, 0, 0, 640, 640,
				  0, CopyFromParent,
				  InputOutput, CopyFromParent,
				  CWBackPixel | CWBorderPixel |
				  CWEventMask | CWColormap,
				  &set_window_attrs);
  XMapWindow(x_display, x_window);
  XFlush(x_display);

  *native_display = (EGLNativeDisplayType)x_display;
  *native_window = (EGLNativeWindowType)x_window;
  return true;
#elif PLATFORM_PI
  bcm_host_init();

  uint32_t width, height;
  int32_t success = graphics_get_display_size(0, &width, &height);
  if (success < 0) {
    fprintf(stderr, "graphics_get_display_size failed\n");
    return false;
  }

  DISPMANX_DISPLAY_HANDLE_T dispman_display = vc_dispmanx_display_open(0);
  DISPMANX_UPDATE_HANDLE_T dispman_update = vc_dispmanx_update_start(0);

  VC_RECT_T dst_rect, src_rect;
  dst_rect.x = 0;
  dst_rect.y = 0;
  dst_rect.width = width;
  dst_rect.height = height;
  src_rect.x = 0;
  src_rect.y = 0;
  src_rect.width = width << 16;
  src_rect.height = height << 16;
  DISPMANX_ELEMENT_HANDLE_T dispman_element =
    vc_dispmanx_element_add(dispman_update, dispman_display,
			    0,  // layer
			    &dst_rect,
			    0,  // src
			    &src_rect,
			    DISPMANX_PROTECTION_NONE,
			    0,  // alpha
			    0,  // clamp
			    DISPMANX_NO_ROTATE  // transform
			    );

  vc_dispmanx_update_submit_sync(dispman_update);

  EGL_DISPMANX_WINDOW_T* egl_window = new EGL_DISPMANX_WINDOW_T;
  egl_window->element = dispman_element;
  egl_window->width = width;
  egl_window->height = height;

  *native_display = EGL_DEFAULT_DISPLAY;
  *native_window = (EGLNativeWindowType)egl_window;
  return true;
#else
  fprintf(stderr, "built without a platform\n");
  return false;
#endif
}

bool init_egl(EGLDisplay* display, EGLSurface* surface, EGLContext* context) {
  EGLNativeDisplayType native_display;
  EGLNativeWindowType window;

  if (!create_window(&native_display, &window)) {
    return false;
  }

  *display = eglGetDisplay(native_display);
  if (*display == EGL_NO_DISPLAY) {
    fprintf(stderr, "eglGetDisplay failed\n");
    return false;
  }

  EGLint major_version, minor_version;
  if (!eglInitialize(*display, &major_version, &minor_version)) {
    fprintf(stderr, "eglInitialize failed\n");
    return false;
  }

  eglBindAPI(EGL_OPENGL_ES_API);
  if (eglGetError() != EGL_SUCCESS) {
    fprintf(stderr, "eglBindAPI failed\n");
    return false;
  }

  EGLint config_attribs[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_NONE,
  };
  EGLConfig config = 0;
  int num_configs = 0;
  if (!eglChooseConfig(*display, config_attribs, &config, 1, &num_configs)) {
    fprintf(stderr, "eglChooseConfig failed\n");
    return false;
  }
  if (num_configs != 1) {
    fprintf(stderr, "eglChooseConfig returned no configs\n");
    return false;
  }

  EGLint context_attribs[] = {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE,
  };
  *context = eglCreateContext(*display, config,
			      EGL_NO_CONTEXT, context_attribs);
  if (eglGetError() != EGL_SUCCESS) {
    fprintf(stderr, "eglCreateContext failed\n");
    return false;
  }

  *surface = eglCreateWindowSurface(*display, config, window, NULL);
  if (eglGetError() != EGL_SUCCESS) {
    fprintf(stderr, "eglCreateWindowSurface failed\n");
    return false;
  }

  eglMakeCurrent(*display, *surface, *surface, *context);
  if (eglGetError() != EGL_SUCCESS) {
    fprintf(stderr, "eglMakeCurrent failed\n");
    return false;
  }

  eglSwapInterval(*display, 1);
  return true;
}

int main(int argc, char** argv) {
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  if (!init_egl(&display, &surface, &context)) {
    return 1;
  }

  // Set background color and clear buffers
  glClearColor(0.15f, 0.25f, 0.35f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  bool done = false;
  while (!done) {
    eglSwapBuffers(display, surface);
  }

  return 0;
}
