// PORTHAUL — animated splash screen (brutalist)
// Plays once at startup: INK bg, a sage bar wipes down, PORT/HAUL letters
// fade+rise in, hold, then the whole thing fades out into the app.
// Drawn via ImGui foreground draw list (no textures). 3x5 stencil font.
#pragma once

#ifndef CLASSIC

#include "ph_palette.h"

#include <imgui.h>

namespace ph::splash {

// 3x5 stencil glyphs (match brand/make_logo.py)
inline char const *const *glyph (char c)
{
	static char const *const P[] = {"111", "101", "111", "100", "100"};
	static char const *const O[] = {"111", "101", "101", "101", "111"};
	static char const *const R[] = {"111", "101", "111", "110", "101"};
	static char const *const T[] = {"111", "010", "010", "010", "010"};
	static char const *const H[] = {"101", "101", "111", "101", "101"};
	static char const *const A[] = {"111", "101", "111", "101", "101"};
	static char const *const U[] = {"101", "101", "101", "101", "111"};
	static char const *const L[] = {"100", "100", "100", "100", "111"};
	switch (c)
	{
	case 'P': return P;
	case 'O': return O;
	case 'R': return R;
	case 'T': return T;
	case 'H': return H;
	case 'A': return A;
	case 'U': return U;
	case 'L': return L;
	}
	return nullptr;
}

// draw a stencil word at (x,y), cell size px, in color col (with alpha)
inline float word (ImDrawList *dl, char const *s, float x, float y, float px, ImU32 col)
{
	for (; *s; ++s)
	{
		auto const *g = glyph (*s);
		if (g)
		{
			for (int r = 0; r < 5; ++r)
				for (int c = 0; c < 3; ++c)
					if (g[r][c] == '1')
						dl->AddRectFilled (ImVec2 (x + c * px, y + r * px),
						    ImVec2 (x + c * px + px, y + r * px + px),
						    col);
		}
		x += 4 * px; // 3 wide + 1 gap
	}
	return x;
}

// Returns true while the splash is still playing (caller should skip/overlay UI).
inline bool draw ()
{
	static float t      = 0.0f;
	static bool finished = false;
	if (finished)
		return false;

	auto const &io = ImGui::GetIO ();
	t += io.DeltaTime;

	constexpr float WIPE = 0.35f; // bar wipes down
	constexpr float RISE = 0.75f; // letters fade+rise done by here
	constexpr float HOLD = 1.6f;  // hold until
	constexpr float FADE = 2.1f;  // fully gone by here

	float const W = io.DisplaySize.x;
	float const H = io.DisplaySize.y;

	auto *const dl = ImGui::GetForegroundDrawList ();

	// global fade-out alpha after HOLD
	float gAlpha = 1.0f;
	if (t > HOLD)
		gAlpha = 1.0f - (t - HOLD) / (FADE - HOLD);
	if (gAlpha <= 0.0f)
	{
		finished = true;
		return false;
	}

	auto A = [&] (float a) -> std::uint8_t {
		float v = a * gAlpha;
		if (v < 0.0f) v = 0.0f;
		if (v > 1.0f) v = 1.0f;
		return static_cast<std::uint8_t> (v * 255.0f);
	};

	// full-screen INK cover
	dl->AddRectFilled (ImVec2 (0, 0), ImVec2 (W, H), pal::imcol (pal::INK, A (1.0f)));

	// layout: centered lockup. cell size scales to screen.
	float const px    = (H < 300 ? 4.0f : 8.0f);    // smaller on 3DS top screen
	float const lockW = 4 * 4 * px;                  // 4 letters * (3+1) cells
	float const lockH = 11 * px;                     // two rows (6 + 5 cells tall)

	// vertical center: on 3DS the top screen is the upper HALF of the canvas,
	// so center the lockup within [0 .. H*0.5]; elsewhere center in full H.
#ifdef __3DS__
	float const bandH = H * 0.5f;
#else
	float const bandH = H;
#endif
	float const topY  = (bandH - lockH) * 0.5f;

	float const barX  = W * 0.5f - lockW * 0.5f - px * 3;
	float const port_y = topY;
	float const haul_y = topY + 6 * px;

    // sage bar wipes DOWN on the left of the lockup
	{
		float const wp   = t < WIPE ? (t / WIPE) : 1.0f;
		float const barH = (6 * px + 5 * px) * wp; // spans both rows
		dl->AddRectFilled (ImVec2 (barX, topY),
		    ImVec2 (barX + px, topY + barH),
		    pal::imcol (pal::SAGE, A (1.0f)));
	}

	// letters fade + rise (PORT then HAUL slightly later)
	float const lx = barX + px * 3;
	{
		float const p = t < RISE ? (t / RISE) : 1.0f;
		float const e = p * p * (3.0f - 2.0f * p);       // smoothstep
		std::uint8_t const la = A (e);
		float const dy = (1.0f - e) * 8.0f;              // rise up
		word (dl, "PORT", lx, port_y + dy, px, pal::imcol (pal::BONE, la));
	}
	{
		float const p = t < (RISE + 0.12f) ? ((t - 0.12f) / RISE) : 1.0f;
		float const pp = p < 0.0f ? 0.0f : p;
		float const e  = pp * pp * (3.0f - 2.0f * pp);
		std::uint8_t const la = A (e);
		float const dy = (1.0f - e) * 8.0f;
		word (dl, "HAUL", lx, haul_y + dy, px, pal::imcol (pal::BONE, la));
	}

	return true;
}

} // namespace ph::splash

#endif // CLASSIC
