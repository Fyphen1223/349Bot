#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkPngEncoder.h"

void SkiaTest(int width, int height,
			  void (*draw)(SkCanvas *),
			  const char *path) {
	sk_sp<SkSurface> rasterSurface =
		SkSurfaces::Raster(SkImageInfo::MakeN32Premul(width, height));
	SkCanvas *rasterCanvas = rasterSurface->getCanvas();
	draw(rasterCanvas);
	sk_sp<SkImage> img(rasterSurface->makeImageSnapshot());
	if (!img) {
		return;
	}
	sk_sp<SkData> png = SkPngEncoder::Encode(nullptr, img.get(), {});
	if (!png) {
		return;
	}
	SkFILEWStream out(path);
	(void) out.write(png->data(), png->size());
}