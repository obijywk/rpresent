#include <stdio.h>
#include <unistd.h>
#include "x11_window.h"

namespace rpresent {

X11Window::X11Window() {
  display_ = XOpenDisplay(NULL);
  if (!display_) {
    fprintf(stderr, "XOpenDisplay failed\n");
    _exit(1);
  }
  long screen = XDefaultScreen(display_);
  Window root_window = RootWindow(display_, screen);

  static const int fb_config_attribs[] = {
    GLX_X_RENDERABLE, True,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_DEPTH_SIZE, 24,
    GLX_DOUBLEBUFFER, True,
    None
  };
  int fb_config_count;
  GLXFBConfig* fb_configs =
    glXChooseFBConfig(display_, screen, fb_config_attribs, &fb_config_count);
  if (!fb_config_count) {
    fprintf(stderr, "glXChooseFBConfig failed\n");
    _exit(1);
  }
  GLXFBConfig fb_config = fb_configs[0];
  XFree(fb_configs);

  glx_context_ = glXCreateNewContext(display_, fb_config,
				     GLX_RGBA_TYPE, NULL, true);
  XSync(display_, false);
  if (!glx_context_) {
    fprintf(stderr, "glXCreateNewContext failed\n");
    _exit(1);
  }

  // int depth = DefaultDepth(display_, screen);
  // XVisualInfo visual_info;
  // XMatchVisualInfo(display_, screen, depth, TrueColor, &visual_info);

  XVisualInfo* visual_info = glXGetVisualFromFBConfig(display_, fb_config);

  XSetWindowAttributes set_window_attrs;
  set_window_attrs.colormap = XCreateColormap(display_, root_window,
					      visual_info->visual, AllocNone);
  set_window_attrs.event_mask = StructureNotifyMask | ExposureMask |
    ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask;

  window_ = XCreateWindow(display_, root_window, 0, 0, 640, 640,
			  0, visual_info->depth,
			  InputOutput, visual_info->visual,
			  CWBackPixel | CWBorderPixel |
			  CWEventMask | CWColormap,
			  &set_window_attrs);
  XMapWindow(display_, window_);
  XFlush(display_);

  glXMakeCurrent(display_, window_, glx_context_);

  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  glXSwapBuffers(display_, window_);
}

X11Window::~X11Window() {
  glXMakeCurrent(display_, 0, 0);
  glXDestroyContext(display_, glx_context_);
  XUnmapWindow(display_, window_);
  XDestroyWindow(display_, window_);
  XCloseDisplay(display_);
}

void X11Window::SwapBuffers() {
  glXSwapBuffers(display_, window_);
}

bool X11Window::ProcessEvents() {
  int num_messages = XPending(display_);
  for (int i = 0; i < num_messages; i++) {
    XEvent event;
    XNextEvent(display_, &event);
    switch (event.type) {
    case ButtonPress:
      return false;
    default:
      break;
    }
  }
  return true;
}

}
