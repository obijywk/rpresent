#include <libxml/nanohttp.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <memory>
#include <stdio.h>
#include <string.h>
#include "image.h"
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

  if (icon_ != VG_INVALID_HANDLE) {
    vgDestroyImage(icon_);
    icon_ = VG_INVALID_HANDLE;
  }
  std::string icon_url =
      std::string("http://www.google.com") +
      EvaluateXPath("/xml_api_reply/weather/current_conditions/icon");
  icon_ = ImageLoader::Load(icon_url);
  if (icon_ == VG_INVALID_HANDLE) {
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
