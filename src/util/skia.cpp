#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/encode/SkPngEncoder.h"

// hv library for HTTP requests
#include "hv/requests.h"

#include <algorithm>// For std::max, std::min
#include <memory>	// For std::unique_ptr
#include <string>
#include <vector>

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

// Function to create an FHD image displaying music info based on the provided style
bool CreateMusicCard(
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
	const char *outputPath) {
	// --- Fixed FHD Resolution ---
	const int width = 1920;
	const int height = 1080;

	// --- Colors ---
	const SkColor bgColor = SkColorSetARGB(0xFF, 0x19, 0x28, 0x41);// Dark Blue
	const SkColor textColor = SK_ColorWHITE;
	const SkColor authorColor = SkColorSetARGB(0xFF, 0x33, 0xA1, 0xFF);		  // Light Blue
	const SkColor secondaryTextColor = SkColorSetARGB(0xFF, 0xC0, 0xC0, 0xC0);// Light Gray
	const SkColor progressBgColor = SkColorSetARGB(0xFF, 0x6C, 0x75, 0x7D);	  // Gray
	const SkColor progressFgColor = SkColorSetARGB(0xFF, 0x76, 0xC7, 0xFF);	  // Lighter Blue
	const SkColor sourceBadgeColor = SkColorSetARGB(0xFF, 0xE6, 0x21, 0x17);  // Red
	const SkColor sourceTextColor = SK_ColorWHITE;

	// --- Layout Constants ---
	const float padding = 60.0f;
	const float thumbnailHeight = height * 0.85f;
	const float thumbnailWidth = thumbnailHeight;// Square thumbnail
	const float thumbnailCornerRadius = 30.0f;
	const float textStartX = thumbnailWidth + 2 * padding;
	const float progressBarHeight = 12.0f;
	const float progressBarY = height - padding - progressBarHeight;
	const float uploaderIconSize = 80.0f;
	const float sourceBadgeHeight = 60.0f;
	const float sourceBadgePadding = 20.0f;

	// 1. Create the drawing surface
	sk_sp<SkSurface> surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(width, height));
	if (!surface) {
		fprintf(stderr, "Failed to create SkSurface.\n");
		return false;
	}
	SkCanvas *canvas = surface->getCanvas();
	canvas->clear(bgColor);

	// 2. Fetch and decode images (Thumbnail and Uploader Icon)
	sk_sp<SkImage> thumbnailImage = FetchAndDecodeImage(thumbnailUrl);
	sk_sp<SkImage> uploaderIconImage = FetchAndDecodeImage(uploaderIconUrl);

	// 3. Draw Thumbnail
	SkPaint paint;
	paint.setAntiAlias(true);
	SkRRect thumbnailRRect = SkRRect::MakeRectXY(
		SkRect::MakeXYWH(padding, (height - thumbnailHeight) / 2.0f, thumbnailWidth, thumbnailHeight),
		thumbnailCornerRadius, thumbnailCornerRadius);

	canvas->save();
	canvas->clipRRect(thumbnailRRect, true);
	if (thumbnailImage) {
		canvas->drawImageRect(thumbnailImage, thumbnailRRect.getBounds(), SkSamplingOptions(SkFilterMode::kLinear));
	} else {
		// Draw placeholder
		paint.setColor(SK_ColorDKGRAY);
		canvas->drawRRect(thumbnailRRect, paint);
		// Optional: Draw error text on placeholder
	}
	canvas->restore();


	// 4. Setup Fonts and Paints for Text
	// Note: Using default font. For specific fonts, load SkTypeface.
	SkFont titleFont;
	titleFont.setSize(80.0f);
	SkFont authorFont;
	authorFont.setSize(60.0f);
	SkFont uploaderFont;
	uploaderFont.setSize(55.0f);
	SkFont sourceFont;
	sourceFont.setSize(40.0f);
	SkFont trackFont;
	trackFont.setSize(70.0f);
	SkFont timeFont;
	timeFont.setSize(50.0f);

	SkPaint textPaint;
	textPaint.setAntiAlias(true);

	// --- Calculate Text Positions ---
	float currentY = padding * 2.5f;

	// 5. Draw Title
	textPaint.setColor(textColor);
	canvas->drawString(title.c_str(), textStartX, currentY, titleFont, textPaint);
	currentY += titleFont.getSize() * 1.1f;

	// 6. Draw Author ("by Author")
	const std::string byStr = "by ";
	SkRect byBounds;
	authorFont.measureText(byStr.c_str(), byStr.length(), SkTextEncoding::kUTF8, &byBounds);
	textPaint.setColor(authorColor);
	canvas->drawString(byStr.c_str(), textStartX, currentY, authorFont, textPaint);
	canvas->drawString(author.c_str(), textStartX + byBounds.width() + 5, currentY, authorFont, textPaint);
	currentY += authorFont.getSize() * 1.5f;

	// 7. Draw Uploader (Icon + Name)
	float uploaderY = currentY;
	float uploaderIconX = textStartX;
	float uploaderIconY = uploaderY - uploaderIconSize * 0.75f;// Adjust vertical alignment
	float uploaderTextX = uploaderIconX + uploaderIconSize + padding * 0.5f;

	if (uploaderIconImage) {
		SkPath circleClip;
		circleClip.addCircle(uploaderIconX + uploaderIconSize / 2.0f, uploaderIconY + uploaderIconSize / 2.0f, uploaderIconSize / 2.0f);
		canvas->save();
		canvas->clipPath(circleClip, true);
		canvas->drawImageRect(uploaderIconImage, SkRect::MakeXYWH(uploaderIconX, uploaderIconY, uploaderIconSize, uploaderIconSize), SkSamplingOptions(SkFilterMode::kLinear));
		canvas->restore();
	} else {
		// Draw placeholder circle
		paint.setColor(SK_ColorDKGRAY);
		paint.setStyle(SkPaint::kFill_Style);
		canvas->drawCircle(uploaderIconX + uploaderIconSize / 2.0f, uploaderIconY + uploaderIconSize / 2.0f, uploaderIconSize / 2.0f, paint);
	}
	textPaint.setColor(textColor);
	canvas->drawString(uploaderName.c_str(), uploaderTextX, uploaderY, uploaderFont, textPaint);
	currentY += uploaderIconSize * 0.5f + padding;// Adjust spacing after uploader line

	// 8. Draw Progress Percentage and Source Badge
	double progressPercent = (totalTimeSec > 0) ? (currentTimeSec / totalTimeSec * 100.0) : 0.0;
	char percentStr[10];
	// Explicitly use double versions of std::min and std::max
	snprintf(percentStr, sizeof(percentStr), "%.0f%%", std::max<double>(0.0, std::min<double>(100.0, progressPercent)));

	SkRect percentBounds;
	sourceFont.measureText(percentStr, strlen(percentStr), SkTextEncoding::kUTF8, &percentBounds);
	SkRect sourceNameBounds;
	sourceFont.measureText(sourceName.c_str(), sourceName.length(), SkTextEncoding::kUTF8, &sourceNameBounds);

	float percentX = textStartX;
	float percentY = currentY;// Align baseline with source badge text
	float badgeX = percentX + percentBounds.width() + padding * 0.5f;
	float badgeWidth = sourceNameBounds.width() + 2 * sourceBadgePadding;
	float badgeY = percentY - sourceBadgeHeight * 0.75f;// Align badge visually

	// Draw Percentage Text
	textPaint.setColor(textColor);
	canvas->drawString(percentStr, percentX, percentY, sourceFont, textPaint);

	// Draw Source Badge Background
	paint.setColor(sourceBadgeColor);
	SkRRect badgeRRect = SkRRect::MakeRectXY(
		SkRect::MakeXYWH(badgeX, badgeY, badgeWidth, sourceBadgeHeight),
		sourceBadgeHeight / 2.0f, sourceBadgeHeight / 2.0f);// Pill shape
	canvas->drawRRect(badgeRRect, paint);

	// Draw Source Name Text on Badge
	textPaint.setColor(sourceTextColor);
	float sourceTextX = badgeX + sourceBadgePadding;
	float sourceTextY = badgeY + sourceBadgeHeight / 2.0f + sourceNameBounds.height() / 2.0f;// Center vertically
	canvas->drawString(sourceName.c_str(), sourceTextX, sourceTextY, sourceFont, textPaint);


	// 9. Draw Track Number (Top Right)
	char trackStr[20];
	snprintf(trackStr, sizeof(trackStr), "%d / %d", currentTrack, totalTracks);
	SkRect trackBounds;
	trackFont.measureText(trackStr, strlen(trackStr), SkTextEncoding::kUTF8, &trackBounds);
	textPaint.setColor(textColor);
	canvas->drawString(trackStr, width - padding - trackBounds.width(), padding * 2.0f, trackFont, textPaint);


	// 10. Draw Progress Bar
	float progressBarWidth = width - 2 * padding;
	// Background
	SkRect progressBarBackRect = SkRect::MakeXYWH(padding, progressBarY, progressBarWidth, progressBarHeight);
	paint.setColor(progressBgColor);
	canvas->drawRect(progressBarBackRect, paint);

	// Foreground
	float progress = (totalTimeSec > 0) ? (currentTimeSec / totalTimeSec) : 0.0f;
	float clampedProgress = std::max(0.0f, std::min(1.0f, progress));
	float progressWidth = progressBarWidth * clampedProgress;
	if (progressWidth > 0) {
		SkRect progressBarFrontRect = SkRect::MakeXYWH(padding, progressBarY, progressWidth, progressBarHeight);
		paint.setColor(progressFgColor);
		canvas->drawRect(progressBarFrontRect, paint);
	}

	// 11. Draw Timestamps
	std::string currentTimeStr = format_time(currentTimeSec);
	std::string totalTimeStr = format_time(totalTimeSec);
	float timeY = progressBarY + progressBarHeight + padding * 0.8f;// Below progress bar

	textPaint.setColor(textColor);
	// Current time (bottom left)
	canvas->drawString(currentTimeStr.c_str(), padding, timeY, timeFont, textPaint);

	// Total time (bottom right)
	SkRect totalTimeBounds;
	timeFont.measureText(totalTimeStr.c_str(), totalTimeStr.length(), SkTextEncoding::kUTF8, &totalTimeBounds);
	canvas->drawString(totalTimeStr.c_str(), width - padding - totalTimeBounds.width(), timeY, timeFont, textPaint);


	// 12. Create final image snapshot
	sk_sp<SkImage> finalImage = surface->makeImageSnapshot();
	if (!finalImage) {
		fprintf(stderr, "Failed to create image snapshot.\n");
		return false;
	}

	// 13. Encode the image as PNG
	sk_sp<SkData> pngData = SkPngEncoder::Encode(nullptr, finalImage.get(), {});
	if (!pngData) {
		fprintf(stderr, "Failed to encode image to PNG.\n");
		return false;
	}

	// 14. Write the PNG data to the output file
	SkFILEWStream outFile(outputPath);
	if (!outFile.isValid()) {
		fprintf(stderr, "Failed to open output file: %s\n", outputPath);
		return false;
	}
	if (!outFile.write(pngData->data(), pngData->size())) {
		fprintf(stderr, "Failed to write PNG data to file: %s\n", outputPath);
		return false;// Writing failed
	}

	return true;// Success
}

/*
// Example Usage (replace with your actual call):
int main() {
	bool success = CreateMusicCardFHD(
		"Tobu - Higher", // title
		"Tobu", // author
		"https://i.scdn.co/image/ab67616d0000b27375a16e71faa8a866966a8ee0", // Example thumbnail URL
		"https://yt3.googleusercontent.com/ytc/AIdro_k-8kZycsKeV_SS86Rk8z7QEzQzNe8Z8AS9YQ=s176-c-k-c0x00ffffff-no-rj", // Example uploader icon URL (F-yphen)
		"Fyphen", // uploaderName
		"YouTube", // sourceName
		60.0, // currentTimeSec (e.g., 1 minute)
		214.0, // totalTimeSec (e.g., 3m34s)
		1, // currentTrack
		2, // totalTracks
		"music_card_fhd.png" // Output file path
	);

	if (success) {
		printf("FHD Music card created successfully: music_card_fhd.png\n");
	} else {
		printf("Failed to create FHD music card.\n");
	}
	return 0;
}
*/