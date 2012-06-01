// Based on http://cgit.freedesktop.org/mesa/demos/tree/src/egl/openvg/text.c

#ifndef RPRESENT_TEXT_H
#define RPRESENT_TEXT_H

#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <VG/openvg.h>

namespace rpresent {

class Text {
 public:
  Text();
  ~Text();

  bool Initialize();
  void DrawString(VGfloat x, VGfloat y, const std::string& str);

 private:
  VGPath ConvertGlyphToPath();

  FT_Library ft_library_;
  FT_Face ft_face_;

  VGFont vg_font_;
};

}  // namespace rpresent

#endif
