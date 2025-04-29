#ifndef SKIA_UTIL_H
#define SKIA_UTIL_H

// Forward declaration to avoid including the full SkCanvas header if not strictly necessary
// However, since the function pointer type uses SkCanvas, including it is clearer.
#include "include/core/SkCanvas.h"

/**
 * @brief Creates a raster surface, draws on it using the provided function,
 *        and saves the result as a PNG image.
 *
 * @param width The width of the image in pixels.
 * @param height The height of the image in pixels.
 * @param draw A function pointer that takes an SkCanvas* and performs drawing operations.
 * @param path The file path where the PNG image will be saved.
 */
void SkiaTest(int width, int height,
			  void (*draw)(SkCanvas *),
			  const char *path);

#endif// SKIA_UTIL_H