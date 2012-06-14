#ifndef RPRESENT_WEATHER_H
#define RPRESENT_WEATHER_H

#include <libxml/tree.h>
#include <string>
#include <VG/openvg.h>

namespace rpresent {

class Weather {
 public:
  explicit Weather(const std::string& city);
  ~Weather();

  bool Refresh();

  std::string City();
  std::string Condition();
  std::string TempF();

  // Weather retains ownership of the image
  VGImage Icon();

 private:
  std::string EvaluateXPath(const std::string& path);

  const std::string city_;
  xmlDocPtr doc_;
  VGImage icon_;
};

}  // namespace rpresent

#endif // RPRESENT_WEATHER_H
