#ifndef SKIA_UTIL_H
#define SKIA_UTIL_H

// Forward declaration to avoid including the full SkCanvas header if not strictly necessary
// However, since the function pointer type uses SkCanvas, including it is clearer.
#include "include/core/SkCanvas.h"

bool CreateMusicCard(const std::string &title,
					 const std::string &author,
					 const std::string &thumbnailUrl,
					 const std::string &uploaderIconUrl,
					 const std::string &uploaderName,
					 const std::string &sourceName,
					 double currentTimeSec,
					 double totalTimeSec,
					 int currentTrack,
					 int totalTracks,
					 const char *outputPath);

#endif// SKIA_UTIL_H