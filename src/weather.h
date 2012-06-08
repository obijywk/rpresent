#ifndef RPRESENT_WEATHER_H
#define RPRESENT_WEATHER_H

#include <libxml/tree.h>
#include <string>
#include <VG/openvg.h>

namespace rpresent {

class Weather {
 public:
  Weather();
  ~Weather();

  bool Initialize(const std::string& city);

  std::string City();
  std::string Condition();
  std::string TempF();

  // Weather retains ownership of the image
  VGImage Icon();

 private:
  std::string EvaluateXPath(const std::string& path);

  // TODO: refactor to an image class
  bool LoadIcon(const std::string& url);

  xmlDocPtr doc_;
  VGImage icon_;
};

}  // namespace rpresent

#endif // RPRESENT_WEATHER_H
