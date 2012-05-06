#include <unistd.h>
#include "rpresenter.h"

namespace rpresent {

RPresenter::RPresenter() {
}

RPresenter::~RPresenter() {
}

void RPresenter::Run() {
  while (window.ProcessEvents()) {
    glClearColor(0, 0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    window.SwapBuffers();
  }
}

}
