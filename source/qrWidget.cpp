// QR connect widget for ftpd fork (descry-ftp)
//
// Renders a QR code via Dear ImGui DrawList (filled rectangles per module).
// See qrWidget.h.

#ifndef CLASSIC

#include "qrWidget.h"

#include "qrcodegen.h"

#include <imgui.h>

#include <algorithm>
#include <cstdint>

namespace qr
{
bool drawQR (std::string const &text_, float pixelSize_)
{
	// generate the QR code into stack buffers
	static std::uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
	static std::uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];

	bool const ok = qrcodegen_encodeText (text_.c_str (),
	    tempBuffer,
	    qrcode,
	    qrcodegen_Ecc_MEDIUM,
	    qrcodegen_VERSION_MIN,
	    qrcodegen_VERSION_MAX,
	    qrcodegen_Mask_AUTO,
	    true);

	if (!ok)
		return false;

	int const modules = qrcodegen_getSize (qrcode); // side length in modules
	if (modules <= 0)
		return false;

	// quiet zone (border) recommended by spec is 4 modules
	constexpr int quiet      = 2; // 2 is plenty on a small screen
	int const totalModules   = modules + 2 * quiet;

	// figure out how big to draw
	float target = pixelSize_;
	if (target <= 0.0f)
	{
		// fit to available content width, capped for the small 3DS screen
		float const avail = ImGui::GetContentRegionAvail ().x;
		target            = std::min (avail, 180.0f);
	}

	// snap module size to an integer number of pixels for crisp edges
	float scale = target / static_cast<float> (totalModules);
	if (scale < 1.0f)
		scale = 1.0f;
	float const px = static_cast<float> (static_cast<int> (scale));
	if (px < 1.0f)
		return false;

	float const dim = px * totalModules;

	// center horizontally in the available region
	float const availX = ImGui::GetContentRegionAvail ().x;
	if (availX > dim)
		ImGui::SetCursorPosX (ImGui::GetCursorPosX () + (availX - dim) * 0.5f);

	ImVec2 const origin = ImGui::GetCursorScreenPos ();
	auto *const drawList = ImGui::GetWindowDrawList ();

	ImU32 const black = IM_COL32 (0, 0, 0, 255);
	ImU32 const white = IM_COL32 (255, 255, 255, 255);

	// white background (includes quiet zone) so it scans on dark themes
	drawList->AddRectFilled (
	    origin, ImVec2 (origin.x + dim, origin.y + dim), white);

	// draw the black modules
	for (int y = 0; y < modules; ++y)
	{
		for (int x = 0; x < modules; ++x)
		{
			if (!qrcodegen_getModule (qrcode, x, y))
				continue;

			float const mx = origin.x + (x + quiet) * px;
			float const my = origin.y + (y + quiet) * px;
			drawList->AddRectFilled (
			    ImVec2 (mx, my), ImVec2 (mx + px, my + px), black);
		}
	}

	// reserve layout space so subsequent widgets flow below the QR
	ImGui::Dummy (ImVec2 (dim, dim));

	return true;
}
}

#endif // CLASSIC
