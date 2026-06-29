// PORTHAUL design system — palette
// brutalist / earthy-military / sharp edges, no rounding.
// SINGLE SOURCE OF TRUTH for color. reused across all projects (the house style).
//
// Colors are declared as explicit (r,g,b). Convert to whatever the backend wants
// via the helpers below. Dear ImGui's IM_COL32(r,g,b,a) packs to 0xAABBGGRR,
// so NEVER hand-write hex literals for ImGui — always go through rgb()/imcol().
#pragma once
#include <cstdint>

namespace ph::pal {

struct RGB { std::uint8_t r, g, b; };

// ---- BASE RAMP (earthy military) ----
constexpr RGB INK      {0x18, 0x1C, 0x14}; // bg, near-black green undertone
constexpr RGB SURFACE  {0x24, 0x28, 0x22}; // panel / window body
constexpr RGB SURFACE2 {0x3C, 0x3D, 0x37}; // raised surface, borders, headers
constexpr RGB MOSS     {0x69, 0x75, 0x65}; // muted text, dim labels, idle accent
constexpr RGB BONE_DIM {0x9A, 0x94, 0x86}; // secondary text
constexpr RGB BONE     {0xEC, 0xDF, 0xCC}; // primary text (warm bone)

// ---- SIGNAL ----
constexpr RGB SAGE      {0x9A, 0xAD, 0x6A}; // mossy sage — transfer/selection/accent
constexpr RGB SAGE_HI   {0xB4, 0xC6, 0x82}; // hover / active (brighter)
constexpr RGB SIGNAL_RED{0xD9, 0x49, 0x3B}; // hard error (burnt red)

// ---- SEMANTIC ROLES (alias the ramp; change here = change everywhere) ----
constexpr RGB BG        = INK;
constexpr RGB PANEL     = SURFACE;
constexpr RGB BORDER    = SURFACE2;
constexpr RGB TEXT      = BONE;
constexpr RGB TEXT_DIM  = MOSS;
constexpr RGB ACCENT    = SAGE;        // active transfer, selection bar, record
constexpr RGB ACCENT_HI = SAGE_HI;
constexpr RGB OK        = SAGE;        // success
constexpr RGB WARN      = SAGE_HI;
constexpr RGB ERR       = SIGNAL_RED;

// ---- CONVERTERS ----
// 0xAABBGGRR — the format IM_COL32 produces (use for ImGui draw lists / style).
constexpr std::uint32_t imcol (RGB c, std::uint8_t a = 0xFF)
{
	return (std::uint32_t (a) << 24) | (std::uint32_t (c.b) << 16) |
	       (std::uint32_t (c.g) << 8) | std::uint32_t (c.r);
}
// 0xRRGGBB — plain, for anything that wants standard hex.
constexpr std::uint32_t hex (RGB c)
{
	return (std::uint32_t (c.r) << 16) | (std::uint32_t (c.g) << 8) | c.b;
}

} // namespace ph::pal
