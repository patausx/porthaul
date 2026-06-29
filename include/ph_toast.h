// PORTHAUL — toast notifications (animated, brutalist)
// Push a short message from anywhere: ph::toast::push("uploaded");
// Call ph::toast::draw() once per frame (after windows) to render + animate.
// Toasts slide up + fade, stack bottom-right, auto-expire. English copy only.
#pragma once

#ifndef CLASSIC

#include "ph_palette.h"

#include <imgui.h>

#include <deque>
#include <string>

namespace ph::toast {

enum class Kind
{
	INFO, // bone
	OK,   // sage accent
	ERR,  // signal red
};

struct Toast
{
	std::string text;
	Kind kind   = Kind::INFO;
	float age    = 0.0f; // seconds shown
	float life   = 2.6f; // total seconds before gone
};

// global queue (tiny; single-threaded UI use)
inline std::deque<Toast> &queue ()
{
	static std::deque<Toast> q;
	return q;
}

inline void push (std::string text_, Kind kind_ = Kind::INFO)
{
	auto &q = queue ();
	q.push_back (Toast{std::move (text_), kind_, 0.0f, 2.6f});
	while (q.size () > 4) // cap stack
		q.pop_front ();
}

inline void ok (std::string t_) { push (std::move (t_), Kind::OK); }
inline void err (std::string t_) { push (std::move (t_), Kind::ERR); }

// smoothstep ease 0..1
inline float ease (float t_)
{
	if (t_ < 0.0f) t_ = 0.0f;
	if (t_ > 1.0f) t_ = 1.0f;
	return t_ * t_ * (3.0f - 2.0f * t_);
}

inline void draw ()
{
	auto &q = queue ();
	if (q.empty ())
		return;

	float const dt = ImGui::GetIO ().DeltaTime;

	auto const &io   = ImGui::GetIO ();
	float const margin = 8.0f;
	float const tw     = 150.0f; // toast width
	float const th     = 22.0f;  // toast height
	float const gap    = 5.0f;

	auto *const dl = ImGui::GetForegroundDrawList ();

	// draw newest at the bottom, older stacked above
	float y = io.DisplaySize.y - margin - th;
	for (auto it = q.rbegin (); it != q.rend (); ++it)
	{
		auto &t = *it;
		t.age += dt;

		// fade in over first 0.18s, fade out over last 0.4s
		float const tin  = ease (t.age / 0.18f);
		float const tout = ease ((t.life - t.age) / 0.4f);
		float const a    = (tin < tout ? tin : tout);

		// slide in from the right by a few px as it appears
		float const slide = (1.0f - tin) * 14.0f;
		float const x     = io.DisplaySize.x - margin - tw + slide;

		pal::RGB bar = pal::SAGE;
		if (t.kind == Kind::ERR) bar = pal::SIGNAL_RED;
		else if (t.kind == Kind::INFO) bar = pal::BONE_DIM;

		ImU32 const bg     = pal::imcol (pal::SURFACE2, static_cast<std::uint8_t> (a * 235));
		ImU32 const barCol = pal::imcol (bar, static_cast<std::uint8_t> (a * 255));
		ImU32 const txt    = pal::imcol (pal::BONE, static_cast<std::uint8_t> (a * 255));

		// brutalist: hard rect panel + a thick accent bar on the left, zero rounding
		dl->AddRectFilled (ImVec2 (x, y), ImVec2 (x + tw, y + th), bg);
		dl->AddRectFilled (ImVec2 (x, y), ImVec2 (x + 4.0f, y + th), barCol);
		dl->AddText (ImVec2 (x + 10.0f, y + 4.0f), txt, t.text.c_str ());

		y -= (th + gap);
	}

	// expire from the front (oldest)
	while (!q.empty () && q.front ().age >= q.front ().life)
		q.pop_front ();
}

} // namespace ph::toast

#endif // CLASSIC
