#ifndef SKIA_H
#define SKIA_H

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/encode/SkPngEncoder.h"
#include "include/ports/SkFontMgr_fontconfig.h"

#include "hv/requests.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>


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
					 const char *outputPath,
					 const char *fontFamily);

#endif// SKIA_H