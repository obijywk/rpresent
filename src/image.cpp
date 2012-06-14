#include <Magick++.h>
#include <memory>
#include "image.h"

namespace rpresent {

VGImage ImageLoader::Load(const std::string& path) {
  Magick::Image image;
  try {
    image.read(path);
  } catch (Magick::Exception& error) {
    fprintf(stderr, "Magick::Image::read failed: %s\n", error.what());
    return VG_INVALID_HANDLE;
  }

  Magick::Geometry geometry = image.size();
  const int width = geometry.width();
  const int height = geometry.height();
  VGImage vg_image = vgCreateImage(VG_sRGBA_8888, width, height,
                                   VG_IMAGE_QUALITY_NONANTIALIASED);
  if (vg_image == VG_INVALID_HANDLE) {
    fprintf(stderr, "vgCreateImage failed: %d\n", vgGetError());
    return VG_INVALID_HANDLE;
  }

  const Magick::PixelPacket* pixels = image.getConstPixels(0, 0, width, height);
  if (sizeof(Magick::PixelPacket) == 8) {
    std::unique_ptr<int[]> data(new int[width * height]);
    for (int y = 0; y < height; y++) {
      int* data_ptr = data.get() + (height - y - 1) * width;
      for (int x = 0; x < width; x++) {
        *data_ptr =
            ((pixels->red & 0xFF00) << 16) |
            ((pixels->green & 0xFF00) << 8) |
            (pixels->blue & 0xFF00) |
            0xFF;
        pixels++;
        data_ptr++;
      }
    }
    vgImageSubData(vg_image, data.get(), width * 4, VG_sRGBA_8888,
                   0, 0, width, height);
  } else if (sizeof(Magick::PixelPacket) == 4) {
    // TODO: fix this case, it doesn't flip the image
    vgImageSubData(vg_image, pixels, width * 4, VG_sRGBA_8888,
                   0, 0, width, height);
  } else {
    fprintf(stderr, "unknown Magick PixelPacket size\n");
    vgDestroyImage(vg_image);
    return VG_INVALID_HANDLE;
  }

  VGErrorCode error_code = vgGetError();
  if (error_code != VG_NO_ERROR) {
    fprintf(stderr, "vgImageSubData failed: %d\n", error_code);
    vgDestroyImage(vg_image);
    return VG_INVALID_HANDLE;
  }

  return vg_image;
}

}  // namespace rpresent
