#ifndef RPRESENT_IMAGE_H
#define RPRESENT_IMAGE_H

#include <string>
#include <VG/openvg.h>

namespace rpresent {

class ImageLoader {
 public:
  static VGImage Load(const std::string& path);

 private:
  ImageLoader();
  ~ImageLoader();
};

}  // namespace rpresent

#endif  // RPRESENT_IMAGE_H
