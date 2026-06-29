// PORTHAUL — Dear ImGui theme (brutalist, earthy-military + signal orange)
// Applies the house design system to the global ImGui style.
// Call ph::theme::apply() once after ImGui::CreateContext().
#pragma once

#ifndef CLASSIC

#include "ph_palette.h"

#include <imgui.h>

namespace ph::theme {

// our RGB -> ImVec4 (0..1)
inline ImVec4 v (pal::RGB c, float a = 1.0f)
{
	return ImVec4 (c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, a);
}

inline void apply (float scale = 1.0f)
{
	auto &style  = ImGui::GetStyle ();
	auto *colors = style.Colors;

	// ---- GEOMETRY: brutalism = sharp, heavy, gridded. ZERO rounding. ----
	style.WindowRounding    = 0.0f;
	style.ChildRounding     = 0.0f;
	style.FrameRounding     = 0.0f;
	style.PopupRounding     = 0.0f;
	style.ScrollbarRounding = 0.0f;
	style.GrabRounding      = 0.0f;
	style.TabRounding       = 0.0f;

	// hard, visible borders — structure on display
	style.WindowBorderSize = 1.0f;
	style.ChildBorderSize  = 1.0f;
	style.FrameBorderSize  = 1.0f;
	style.PopupBorderSize  = 1.0f;
	style.TabBorderSize    = 0.0f;

	// tight, deliberate spacing on a small screen
	style.WindowPadding   = ImVec2 (6, 6);
	style.FramePadding    = ImVec2 (6, 3);
	style.ItemSpacing     = ImVec2 (6, 4);
	style.ItemInnerSpacing= ImVec2 (4, 4);
	style.ScrollbarSize   = 10.0f;
	style.GrabMinSize      = 10.0f;
	style.WindowTitleAlign = ImVec2 (0.0f, 0.5f); // left-aligned titles, like a label

	// ---- COLOR ----
	colors[ImGuiCol_Text]                 = v (pal::TEXT);
	colors[ImGuiCol_TextDisabled]         = v (pal::TEXT_DIM);
	colors[ImGuiCol_WindowBg]             = v (pal::BG);
	colors[ImGuiCol_ChildBg]              = v (pal::BG);
	colors[ImGuiCol_PopupBg]              = v (pal::SURFACE);
	colors[ImGuiCol_Border]               = v (pal::BORDER);
	colors[ImGuiCol_BorderShadow]         = v (pal::BG, 0.0f);

	colors[ImGuiCol_FrameBg]              = v (pal::SURFACE2);
	colors[ImGuiCol_FrameBgHovered]       = v (pal::MOSS, 0.5f);
	colors[ImGuiCol_FrameBgActive]        = v (pal::MOSS, 0.7f);

	colors[ImGuiCol_TitleBg]              = v (pal::SURFACE2);
	colors[ImGuiCol_TitleBgActive]        = v (pal::SURFACE2);
	colors[ImGuiCol_TitleBgCollapsed]     = v (pal::SURFACE);
	colors[ImGuiCol_MenuBarBg]            = v (pal::SURFACE2);

	colors[ImGuiCol_ScrollbarBg]          = v (pal::BG);
	colors[ImGuiCol_ScrollbarGrab]        = v (pal::SURFACE2);
	colors[ImGuiCol_ScrollbarGrabHovered] = v (pal::MOSS);
	colors[ImGuiCol_ScrollbarGrabActive]  = v (pal::ACCENT);

	colors[ImGuiCol_CheckMark]            = v (pal::ACCENT);
	colors[ImGuiCol_SliderGrab]           = v (pal::MOSS);
	colors[ImGuiCol_SliderGrabActive]     = v (pal::ACCENT);

	// buttons: flat surface, orange on press — the signal
	colors[ImGuiCol_Button]               = v (pal::SURFACE2);
	colors[ImGuiCol_ButtonHovered]        = v (pal::MOSS);
	colors[ImGuiCol_ButtonActive]         = v (pal::ACCENT);

	// headers / selectables / menu items: orange selection bar feel
	colors[ImGuiCol_Header]               = v (pal::SURFACE2);
	colors[ImGuiCol_HeaderHovered]        = v (pal::MOSS);
	colors[ImGuiCol_HeaderActive]         = v (pal::ACCENT);

	colors[ImGuiCol_Separator]            = v (pal::BORDER);
	colors[ImGuiCol_SeparatorHovered]     = v (pal::MOSS);
	colors[ImGuiCol_SeparatorActive]      = v (pal::ACCENT);

	colors[ImGuiCol_ResizeGrip]           = v (pal::SURFACE2, 0.0f);
	colors[ImGuiCol_ResizeGripHovered]    = v (pal::MOSS);
	colors[ImGuiCol_ResizeGripActive]     = v (pal::ACCENT);

	colors[ImGuiCol_Tab]                  = v (pal::SURFACE);
	colors[ImGuiCol_TabHovered]           = v (pal::MOSS);
	colors[ImGuiCol_TabSelected]          = v (pal::SURFACE2);
	colors[ImGuiCol_TabSelectedOverline]  = v (pal::ACCENT);
	colors[ImGuiCol_TabDimmed]            = v (pal::SURFACE);
	colors[ImGuiCol_TabDimmedSelected]    = v (pal::SURFACE2);

	// data viz: transfers glow orange
	colors[ImGuiCol_PlotLines]            = v (pal::ACCENT);
	colors[ImGuiCol_PlotLinesHovered]     = v (pal::ACCENT_HI);
	colors[ImGuiCol_PlotHistogram]        = v (pal::ACCENT);
	colors[ImGuiCol_PlotHistogramHovered] = v (pal::ACCENT_HI);

	colors[ImGuiCol_TableHeaderBg]        = v (pal::SURFACE2);
	colors[ImGuiCol_TableBorderStrong]    = v (pal::BORDER);
	colors[ImGuiCol_TableBorderLight]     = v (pal::SURFACE);
	colors[ImGuiCol_TableRowBg]           = v (pal::BG, 0.0f);
	colors[ImGuiCol_TableRowBgAlt]        = v (pal::SURFACE, 0.4f);

	colors[ImGuiCol_TextSelectedBg]       = v (pal::ACCENT, 0.5f);
	colors[ImGuiCol_NavCursor]            = v (pal::ACCENT);
	colors[ImGuiCol_DragDropTarget]       = v (pal::ACCENT);
	colors[ImGuiCol_ModalWindowDimBg]     = v (pal::INK, 0.6f);

	// scale all geometry for small screens (3DS uses 0.5). Colors are unaffected.
	if (scale != 1.0f)
		style.ScaleAllSizes (scale);
}

} // namespace ph::theme

#endif // CLASSIC
