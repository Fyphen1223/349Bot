#include "./skia.h"


sk_sp<SkImage> FetchAndDecodeImage(const std::string &url) {
	auto resp = requests::get(url.c_str());

	if (!resp || resp->status_code != 200 || resp->body.empty()) {
		fprintf(stderr, "Failed to fetch or empty body for URL: %s\n", url.c_str());
		if (resp) {
			fprintf(stderr, "Status code: %d\n", resp->status_code);
		}
		return nullptr;// Return null if request fails or body is empty
	}

	sk_sp<SkData> imageData = SkData::MakeWithCopy(resp->body.data(), resp->body.size());
	if (!imageData) {
		fprintf(stderr, "Failed to create SkData from response body.\n");
		return nullptr;
	}

	std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(imageData);
	if (!codec) {
		fprintf(stderr, "Failed to create SkCodec from image data for URL: %s\n", url.c_str());
		return nullptr;
	}

	SkBitmap bitmap;
	SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
	// Allocate pixels for the bitmap based on the info.
	bitmap.allocPixels(info);
	// Check if pixel allocation was successful.
	if (!bitmap.getPixels()) {
		fprintf(stderr, "Failed to allocate pixels for bitmap.\n");
		return nullptr;
	}
	// Attempt to decode the image pixels into the allocated bitmap memory.
	if (codec->getPixels(info, bitmap.getPixels(), bitmap.rowBytes()) != SkCodec::kSuccess) {
		fprintf(stderr, "Failed to decode image pixels for URL: %s\n", url.c_str());
		return nullptr;
	}
	bitmap.setImmutable();
	return SkImages::RasterFromBitmap(bitmap);
}

#include "include/effects/SkImageFilters.h"
#include "include/pathops/SkPathOps.h"
#include "skia.h"
#include <chrono>
#include <future>
#include <mutex>
#include <unordered_map>


// Function to format time in seconds to M:SS or S s format
std::string format_time(double total_seconds) {
	if (total_seconds < 0)
		return "0:00";
	int seconds = static_cast<int>(total_seconds);
	int minutes = seconds / 60;
	seconds %= 60;
	if (minutes > 0) {
		char buffer[10];
		snprintf(buffer, sizeof(buffer), "%d:%02d", minutes, seconds);
		return std::string(buffer);
	} else {
		char buffer[10];
		snprintf(buffer, sizeof(buffer), "%ds", seconds);
		return std::string(buffer);
	}
}

sk_sp<SkFontMgr> GetFontMgr() {
	static sk_sp<SkFontMgr> mgr = SkFontMgr_New_FontConfig(nullptr);
	return mgr;
}

sk_sp<SkTypeface> GetTypeface(const char *fontFamily) {
	static std::unordered_map<std::string, sk_sp<SkTypeface>> cache;
	static std::mutex cache_mutex;
	std::string key(fontFamily ? fontFamily : "");
	{
		std::lock_guard<std::mutex> lock(cache_mutex);
		auto it = cache.find(key);
		if (it != cache.end())
			return it->second;
	}
	sk_sp<SkTypeface> tf = GetFontMgr()->legacyMakeTypeface(fontFamily, SkFontStyle(400, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
	if (tf) {
		std::lock_guard<std::mutex> lock(cache_mutex);
		cache[key] = tf;
	}
	return tf;
}


// --- Fixed FHD Resolution ---s
const int width = 1920;
const int height = 1080;

// --- Colors ---
const SkColor bgColor = SkColorSetRGB(255, 255, 255);// White
const SkColor textColor = SK_ColorBLACK;
const SkColor authorColor = SkColorSetARGB(0xFF, 0x33, 0xA1, 0xFF);		  // Light Blue
const SkColor secondaryTextColor = SkColorSetARGB(0xFF, 0xC0, 0xC0, 0xC0);// Light Gray
const SkColor progressBgColor = SkColorSetARGB(0xFF, 0x6C, 0x75, 0x7D);	  // Gray
const SkColor progressFgColor = SkColorSetARGB(0xFF, 0x76, 0xC7, 0xFF);	  // Lighter Blue
const SkColor sourceBadgeColor = SkColorSetARGB(0xFF, 0xE6, 0x21, 0x17);  // Red
const SkColor sourceTextColor = SK_ColorBLACK;

// --- Layout Constants ---
const float padding = 60.0f;
const float thumbnailHeight = height * 0.85f;
const float thumbnailWidth = thumbnailHeight;// Square thumbnail
const float thumbnailCornerRadius = 30.0f;
const float textStartX = thumbnailWidth + 2 * padding;
const float progressBarHeight = 12.0f;
const float progressBarY = height - padding - progressBarHeight;
const float uploaderIconSize = 110.0f;
const float sourceBadgeHeight = 60.0f;
const float sourceBadgePadding = 20.0f;

sk_sp<SkData> CreateMusicCard(
	const std::string &title,
	const std::string &author,
	const std::string &thumbnailUrl,
	const std::string &uploaderIconUrl,
	const std::string &uploaderName,
	const std::string &sourceName,
	double currentTimeSec,
	double totalTimeSec,
	int currentTrack,
	int totalTracks,
	const char *fontFamily) {
	auto start = std::chrono::high_resolution_clock::now();
	sk_sp<SkTypeface> typeface = GetTypeface(fontFamily);
	if (!typeface) {
		fprintf(stderr, "Failed to load typeface: %s\n", fontFamily);
	}

	// 画像取得を非同期で開始
	auto thumbFuture = std::async(std::launch::async, FetchAndDecodeImage, thumbnailUrl);
	auto iconFuture = std::async(std::launch::async, FetchAndDecodeImage, uploaderIconUrl);

	// 1. 描画サーフェス作成
	sk_sp<SkSurface> surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(width, height));
	SkCanvas *canvas = surface->getCanvas();

	// 背景塗りつぶし
	canvas->clear(bgColor);

	// 2. まずテキストや図形だけ描画（画像以外）
	SkFont titleFont(typeface, 120.0f);
	SkFont uploaderFont(typeface, 90.0f);
	SkFont sourceFont(typeface, 50.0f);
	SkFont trackFont(typeface, 80.0f);
	SkFont timeFont(typeface, 50.0f);

	SkPaint textPaint;
	textPaint.setAntiAlias(true);

	SkPaint paint;
	paint.setAntiAlias(true);

	// サムネイル枠だけ先に描画（画像は後で）
	SkRRect thumbnailRRect = SkRRect::MakeRectXY(
		SkRect::MakeXYWH(padding, (height - thumbnailHeight) / 2.0f, thumbnailWidth, thumbnailHeight),
		thumbnailCornerRadius, thumbnailCornerRadius);
	paint.setColor(SK_ColorLTGRAY);
	paint.setStyle(SkPaint::kFill_Style);
	canvas->drawRRect(thumbnailRRect, paint);

	// --- テキストや図形の描画 ---
	float currentY = padding * 5.5f;
	// タイトル
	textPaint.setColor(textColor);
	canvas->drawString(title.c_str(), textStartX, currentY, titleFont, textPaint);
	currentY += titleFont.getSize() * 1.1f;

	// アップローダー名（アイコンは後で）
	float uploaderY = currentY;
	float uploaderIconX = textStartX;
	float uploaderIconY = uploaderY - uploaderIconSize * 0.75f;
	float uploaderTextX = uploaderIconX + uploaderIconSize + padding * 0.5f;

	textPaint.setColor(textColor);
	canvas->drawString(uploaderName.c_str(), uploaderTextX, uploaderY, uploaderFont, textPaint);
	currentY += uploaderIconSize * 0.5f + padding;

	// 進捗パーセントとバッジ
	double progressPercent = (totalTimeSec > 0) ? (currentTimeSec / totalTimeSec * 100.0) : 0.0;
	char percentStr[10];
	snprintf(percentStr, sizeof(percentStr), "%.0f%%", std::max<double>(0.0, std::min<double>(100.0, progressPercent)));

	SkRect percentBounds;
	sourceFont.measureText(percentStr, strlen(percentStr), SkTextEncoding::kUTF8, &percentBounds);
	SkRect sourceNameBounds;
	sourceFont.measureText(sourceName.c_str(), sourceName.length(), SkTextEncoding::kUTF8, &sourceNameBounds);

	float percentX = textStartX;
	float percentY = currentY;
	float badgeX = percentX + percentBounds.width() + padding * 0.5f;
	float badgeWidth = sourceNameBounds.width() + 2 * sourceBadgePadding;
	float badgeY = percentY - sourceBadgeHeight * 0.75f;

	textPaint.setColor(textColor);
	canvas->drawString(percentStr, percentX, percentY, sourceFont, textPaint);

	paint.setColor(sourceBadgeColor);
	SkRRect badgeRRect = SkRRect::MakeRectXY(
		SkRect::MakeXYWH(badgeX, badgeY, badgeWidth, sourceBadgeHeight),
		sourceBadgeHeight / 2.0f, sourceBadgeHeight / 2.0f);
	canvas->drawRRect(badgeRRect, paint);

	textPaint.setColor(sourceTextColor);
	float sourceTextX = badgeX + sourceBadgePadding;
	float sourceTextY = badgeY + sourceBadgeHeight / 2.0f + sourceNameBounds.height() / 2.0f;
	canvas->drawString(sourceName.c_str(), sourceTextX, sourceTextY, sourceFont, textPaint);

	// Track number
	char trackStr[20];
	snprintf(trackStr, sizeof(trackStr), "%d / %d", currentTrack, totalTracks);
	SkRect trackBounds;
	trackFont.measureText(trackStr, strlen(trackStr), SkTextEncoding::kUTF8, &trackBounds);
	textPaint.setColor(textColor);
	canvas->drawString(trackStr, width - padding - trackBounds.width(), padding * 2.0f, trackFont, textPaint);

	// Progress bar
	float progressBarWidth = width - 2 * padding;
	SkRect progressBarBackRect = SkRect::MakeXYWH(padding, progressBarY, progressBarWidth, progressBarHeight);
	paint.setColor(progressBgColor);
	canvas->drawRect(progressBarBackRect, paint);

	float progress = (totalTimeSec > 0) ? (currentTimeSec / totalTimeSec) : 0.0f;
	float clampedProgress = std::max(0.0f, std::min(1.0f, progress));
	float progressWidth = progressBarWidth * clampedProgress;
	if (progressWidth > 0) {
		SkRect progressBarFrontRect = SkRect::MakeXYWH(padding, progressBarY, progressWidth, progressBarHeight);
		paint.setColor(progressFgColor);
		canvas->drawRect(progressBarFrontRect, paint);
	}

	// Timestamp
	std::string currentTimeStr = format_time(currentTimeSec);
	std::string totalTimeStr = format_time(totalTimeSec);
	float timeY = progressBarY + progressBarHeight + padding * 0.8f;
	textPaint.setColor(textColor);
	canvas->drawString(currentTimeStr.c_str(), padding, timeY, timeFont, textPaint);
	SkRect totalTimeBounds;
	timeFont.measureText(totalTimeStr.c_str(), totalTimeStr.length(), SkTextEncoding::kUTF8, &totalTimeBounds);
	canvas->drawString(totalTimeStr.c_str(), width - padding - totalTimeBounds.width(), timeY, timeFont, textPaint);

	// Wait for the images to be fetched
	sk_sp<SkImage> thumbnailImage = thumbFuture.get();
	sk_sp<SkImage> uploaderIconImage = iconFuture.get();

	// Draw thumbnail image
	if (thumbnailImage) {
		canvas->save();
		canvas->clipRRect(thumbnailRRect, true);
		canvas->drawImageRect(thumbnailImage, thumbnailRRect.getBounds(), SkSamplingOptions(SkFilterMode::kLinear));
		canvas->restore();
	}

	// Draw uploader's icon
	if (uploaderIconImage) {
		SkPath circleClip;
		circleClip.addCircle(uploaderIconX + uploaderIconSize / 2.0f, uploaderIconY + uploaderIconSize / 2.0f, uploaderIconSize / 2.0f);
		canvas->save();
		canvas->clipPath(circleClip, true);
		canvas->drawImageRect(uploaderIconImage, SkRect::MakeXYWH(uploaderIconX, uploaderIconY, uploaderIconSize, uploaderIconSize), SkSamplingOptions(SkFilterMode::kLinear));
		canvas->restore();
	} else {
		// placeholder
		paint.setColor(SK_ColorDKGRAY);
		paint.setStyle(SkPaint::kFill_Style);
		canvas->drawCircle(uploaderIconX + uploaderIconSize / 2.0f, uploaderIconY + uploaderIconSize / 2.0f, uploaderIconSize / 2.0f, paint);
	}

	sk_sp<SkImage> finalImage = surface->makeImageSnapshot();
	sk_sp<SkData> pngData = SkPngEncoder::Encode(nullptr, finalImage.get(), {});
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	fprintf(stderr, "Image generation took %lld ms\n", duration.count());
	return pngData;
}