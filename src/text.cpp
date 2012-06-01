#include <freetype/ftoutln.h>
#include <stdio.h>
#include <memory>
#include "text.h"

namespace rpresent {

namespace {

const VGfloat kFontSize = 64.0f;

void AppendFTVectorsToPath(VGPath path,
                           VGubyte segment,
                           const FT_Vector** vectors) {
  VGfloat coords[6];
  int num_vectors;

  switch (segment) {
    case VG_MOVE_TO:
    case VG_LINE_TO:
      num_vectors = 1;
      break;
    case VG_QUAD_TO:
      num_vectors = 2;
      break;
    case VG_CUBIC_TO:
      num_vectors = 3;
      break;
    default:
      fprintf(stderr, "Unknown VG segment type: %d\n", segment);
      return;
  }

  for (int i = 0; i < num_vectors; i++) {
    coords[2 * i + 0] = static_cast<VGfloat>(vectors[i]->x) / kFontSize;
    coords[2 * i + 1] = static_cast<VGfloat>(vectors[i]->y) / kFontSize;
  }

  vgAppendPathData(path, 1, &segment, static_cast<const void*>(coords));
}

int FTOutlineMoveTo(const FT_Vector* to, void* user) {
  VGPath path = static_cast<VGPath>(reinterpret_cast<size_t>(user));
  AppendFTVectorsToPath(path, VG_MOVE_TO, &to);
  return 0;
}

int FTOutlineLineTo(const FT_Vector* to, void* user) {
  VGPath path = static_cast<VGPath>(reinterpret_cast<size_t>(user));
  AppendFTVectorsToPath(path, VG_LINE_TO, &to);
  return 0;
}

int FTOutlineConicTo(const FT_Vector* control,
                     const FT_Vector* to,
                     void* user) {
  VGPath path = static_cast<VGPath>(reinterpret_cast<size_t>(user));
  const FT_Vector* vectors[] = { control, to };
  AppendFTVectorsToPath(path, VG_QUAD_TO, vectors);
  return 0;
}

int FTOutlineCubicTo(const FT_Vector* control1,
                     const FT_Vector* control2,
                     const FT_Vector* to,
                     void* user) {
  VGPath path = static_cast<VGPath>(reinterpret_cast<size_t>(user));
  const FT_Vector* vectors[] = { control1, control2, to };
  AppendFTVectorsToPath(path, VG_CUBIC_TO, vectors);
  return 0;
}

}  // namespace

Text::Text()
  : ft_library_(0),
    ft_face_(0),
    vg_font_(VG_INVALID_HANDLE) {
}

Text::~Text() {
  if (vg_font_ != VG_INVALID_HANDLE) {
    vgDestroyFont(vg_font_);
  }
  if (ft_face_) {
    FT_Done_Face(ft_face_);
  }
  if (ft_library_) {
    FT_Done_FreeType(ft_library_);
  }
}

bool Text::Initialize() {
  if (FT_Init_FreeType(&ft_library_)) {
    fprintf(stderr, "FT_Init_FreeType failed\n");
    return false;
  }
  if (FT_New_Face(ft_library_,
                  // "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
                  "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf",
                  0,
		  &ft_face_)) {
    fprintf(stderr, "FT_New_Face failed\n");
    return false;
  }
  if (FT_Select_Charmap(ft_face_, FT_ENCODING_UNICODE)) {
    fprintf(stderr, "FT_Select_Charmap failed\n");
    return false;
  }
  if (FT_Set_Pixel_Sizes(ft_face_,
                         static_cast<FT_UInt>(kFontSize),
                         static_cast<FT_UInt>(kFontSize))) {
    fprintf(stderr, "FT_Set_Pixel_Sizes failed\n");
    return false;
  }

  vg_font_ = vgCreateFont(128);
  if (vg_font_ == VG_INVALID_HANDLE) {
    fprintf(stderr, "vgCreateFont failed: %d\n", vgGetError());
    return false;
  }

  const VGfloat origin[] = { 0.0f, 0.0f };
  VGfloat escapement[2];
  VGErrorCode vg_error;
  for (FT_ULong c = 1; c < 128; c++) {
    if (FT_Load_Char(ft_face_, c, FT_LOAD_DEFAULT)) {
      fprintf(stderr, "FT_Load_Char failed: %lu\n", c);
      continue;
    }
    if (ft_face_->glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
      fprintf(stderr, "FT unsupported glyph format: %d\n",
	      ft_face_->glyph->format);
      continue;
    }

    escapement[0] =
        static_cast<VGfloat>(ft_face_->glyph->advance.x) / kFontSize;
    escapement[1] =
        static_cast<VGfloat>(ft_face_->glyph->advance.y) / kFontSize;

    VGPath char_path = ConvertGlyphToPath();
    if (char_path == VG_INVALID_HANDLE) {
      continue;
    }
    vgSetGlyphToPath(vg_font_, c, char_path, VG_TRUE, origin, escapement);
    vg_error = vgGetError();
    if (vg_error != VG_NO_ERROR) {
      fprintf(stderr, "vgSetGlyphToPath failed: %d\n", vg_error);
    }
  }

  return true;
}

void Text::DrawString(VGfloat x, VGfloat y, const std::string& str) {
  std::unique_ptr<VGuint[]> indices(new VGuint[str.length()]);
  int i = 0;
  for (const char& c : str) {
    indices[i++] = (VGuint)c;
  }

  const VGfloat origin[] = { x, y };
  vgSetfv(VG_GLYPH_ORIGIN, 2, origin);
  vgDrawGlyphs(vg_font_, str.length(), indices.get(),
               NULL, NULL, VG_FILL_PATH, VG_TRUE);
}

VGPath Text::ConvertGlyphToPath() {
  static const FT_Outline_Funcs funcs = {
    &FTOutlineMoveTo,
    &FTOutlineLineTo,
    &FTOutlineConicTo,
    &FTOutlineCubicTo,
    0, 0
  };

  VGPath path = vgCreatePath(
    VG_PATH_FORMAT_STANDARD,
    VG_PATH_DATATYPE_F,
    1.0f, 0.0f,
    0, ft_face_->glyph->outline.n_points,
    VG_PATH_CAPABILITY_ALL);
  if (FT_Outline_Decompose(&ft_face_->glyph->outline,
                           &funcs,
                           reinterpret_cast<void*>(path))) {
    fprintf(stderr, "FT_Outline_Decompose failed\n");
    vgDestroyPath(path);
    return VG_INVALID_HANDLE;
  }
  return path;
}

}  // namespace rpresent
