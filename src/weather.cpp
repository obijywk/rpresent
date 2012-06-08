#include <Magick++.h>
#include <libxml/nanohttp.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <memory>
#include <stdio.h>
#include <string.h>
#include "weather.h"

namespace rpresent {

Weather::Weather()
    : doc_(NULL),
      icon_(VG_INVALID_HANDLE) {
}

Weather::~Weather() {
  if (doc_) {
    xmlFreeDoc(doc_);
  }
  if (icon_ != VG_INVALID_HANDLE) {
    vgDestroyImage(icon_);
  }
}

bool Weather::Initialize(const std::string& city) {
  // TODO: escape city
  const std::string url =
      std::string("http://www.google.com/ig/api?weather=") + city;
  std::unique_ptr<void> context(xmlNanoHTTPOpen(url.c_str(), NULL));
  if (context.get() == NULL) {
    fprintf(stderr, "xmlNanoHTTPOpen failed\n");
    return false;
  }

  // TODO: better buffer size logic
  std::unique_ptr<char[]> buffer(new char[4096]);
  int bytes_read = xmlNanoHTTPRead(context.get(), buffer.get(), 4096);
  if (bytes_read == -1) {
    fprintf(stderr, "xmlNanoHTTPRead failed due to parameter error\n");
    return false;
  }

  doc_ = xmlReadMemory(buffer.get(), strlen(buffer.get()),
                       url.c_str(), NULL, 0);
  if (doc_ == NULL) {
    fprintf(stderr, "xmlReadMemory failed\n");
    return false;
  }

  std::string icon_url =
      std::string("http://www.google.com") +
      EvaluateXPath("/xml_api_reply/weather/current_conditions/icon");
  if (!LoadIcon(icon_url)) {
    return false;
  }

  return true;
}

bool Weather::LoadIcon(const std::string& url) {
  if (icon_ != VG_INVALID_HANDLE) {
    vgDestroyImage(icon_);
    icon_ = VG_INVALID_HANDLE;
  }

  Magick::Image image(url);
  Magick::Geometry geometry = image.size();
  icon_ = vgCreateImage(VG_sRGBA_8888, geometry.width(), geometry.height(),
                        VG_IMAGE_QUALITY_NONANTIALIASED);
  if (icon_ == VG_INVALID_HANDLE) {
    fprintf(stderr, "vgCreateImage failed: %d\n", vgGetError());
    return false;
  }

  const Magick::PixelPacket* pixels =
      image.getConstPixels(0, 0, geometry.width(), geometry.height());

  if (sizeof(Magick::PixelPacket) == 8) {
    std::unique_ptr<int[]> data(
      new int[geometry.width() * geometry.height()]);
    for (int y = 0; y < geometry.height(); y++) {
      for (int x = 0; x < geometry.width(); x++) {
        const Magick::PixelPacket* packet = pixels + y*geometry.width() + x;
        data[(geometry.height() - y - 1) * geometry.width() + x] =
            ((packet->red >> 8) << 24) + ((packet->green >> 8) << 16) +
            ((packet->blue >> 8) << 8) + 0xFF;
      }
    }
    vgImageSubData(icon_, data.get(), geometry.width() * 4, VG_sRGBA_8888,
                   0, 0, geometry.width(), geometry.height());
  } else if (sizeof(Magick::PixelPacket) == 4) {
    vgImageSubData(icon_, pixels, geometry.width() * 4, VG_sRGBA_8888,
                   0, 0, geometry.width(), geometry.height());
  } else {
    fprintf(stderr, "unknown Magick PixelPacket size\n");
    return false;
  }

  VGErrorCode error_code = vgGetError();
  if (error_code != VG_NO_ERROR) {
    fprintf(stderr, "vgImageSubData failed: %d\n", error_code);
    return false;
  }

  return true;
}

VGImage Weather::Icon() {
  return icon_;
}

std::string Weather::EvaluateXPath(const std::string& path) {
  if (doc_ == NULL) {
    return "";
  }
  xmlXPathContextPtr context = xmlXPathNewContext(doc_);
  if (context == NULL) {
    return "";
  }
  xmlXPathObjectPtr object = xmlXPathEvalExpression(
    reinterpret_cast<const xmlChar*>(path.c_str()), context);
  if (object == NULL ||
      object->nodesetval->nodeNr == 0 ||
      object->nodesetval->nodeTab[0]->properties == NULL) {
    xmlXPathFreeContext(context);
    return "";
  }
  std::string data = reinterpret_cast<const char*>(
    object->nodesetval->nodeTab[0]->properties->children->content);
  xmlXPathFreeObject(object);
  xmlXPathFreeContext(context);
  return data;
}

std::string Weather::City() {
  return EvaluateXPath("/xml_api_reply/weather/forecast_information/city");
}

std::string Weather::Condition() {
  return EvaluateXPath("/xml_api_reply/weather/current_conditions/condition");
}

std::string Weather::TempF() {
  return EvaluateXPath("/xml_api_reply/weather/current_conditions/temp_f");
}


}  // namespace rpresent
