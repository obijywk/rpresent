#ifndef RPRESENTER_RPRESENTER_H
#define RPRESENTER_RPRESENTER_H

#include "x11_window.h"

namespace rpresent {

class RPresenter {
 public:
  RPresenter();
  ~RPresenter();

  void Run();

 private:
  X11Window window;
};

}

#endif
