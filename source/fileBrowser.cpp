// PORTHAUL — local file browser widget. See fileBrowser.h.
#ifndef CLASSIC

#include "fileBrowser.h"

#include "fs.h"
#include "log.h"
#include "ph_palette.h"

#include <imgui.h>

#include <dirent.h>
#include <sys/stat.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace ph {

namespace {
#if defined(__3DS__) || defined(__SWITCH__)
constexpr char const *ROOT = "sdmc:/";
#else
constexpr char const *ROOT = "/";
#endif
} // namespace

FileBrowser::FileBrowser () : m_path (ROOT) {}

std::string FileBrowser::fullPath (std::string const &name_) const
{
	std::string full = m_path;
	if (!full.empty () && full.back () != '/')
		full += '/';
	full += name_;
	return full;
}

void FileBrowser::refresh ()
{
	m_entries.clear ();
	m_selected = -1;
	m_dirty    = false;

	DIR *dir = ::opendir (m_path.c_str ());
	if (!dir)
		return;

	while (auto *const ent = ::readdir (dir))
	{
		std::string const name = ent->d_name;
		if (name == "." || name == "..")
			continue;

		Entry e;
		e.name = name;

		struct stat st{};
		if (::stat (fullPath (name).c_str (), &st) == 0)
		{
			e.isDir = S_ISDIR (st.st_mode);
			e.size  = static_cast<std::uint64_t> (st.st_size);
		}
		m_entries.push_back (std::move (e));
	}
	::closedir (dir);

	std::sort (m_entries.begin (), m_entries.end (), [] (Entry const &a, Entry const &b) {
		if (a.isDir != b.isDir)
			return a.isDir > b.isDir;
		return std::lexicographical_compare (a.name.begin (),
		    a.name.end (),
		    b.name.begin (),
		    b.name.end (),
		    [] (char x, char y) { return std::tolower (x) < std::tolower (y); });
	});
}

void FileBrowser::enter (std::string const &name_)
{
	if (name_ == "..")
	{
		std::string p = m_path;
		if (p.size () > 1 && p.back () == '/')
			p.pop_back ();
		auto const slash = p.find_last_of ('/');
		if (slash != std::string::npos && slash >= std::string (ROOT).size () - 1)
			p = p.substr (0, slash + 1);
		else
			p = ROOT;
		m_path = p;
	}
	else
	{
		if (!m_path.empty () && m_path.back () != '/')
			m_path += '/';
		m_path += name_;
		m_path += '/';
	}
	m_dirty = true;
}

void FileBrowser::doDelete ()
{
	if (m_selected < 0 || m_selected >= static_cast<int> (m_entries.size ()))
		return;

	auto const &e   = m_entries[m_selected];
	auto const full = fullPath (e.name);

	int rc = e.isDir ? ::rmdir (full.c_str ()) : ::unlink (full.c_str ());
	if (rc != 0)
		error ("delete failed: %s\n", e.name.c_str ());
	else
		info ("deleted %s\n", e.name.c_str ());

	m_dirty = true;
}

void FileBrowser::draw ()
{
	if (m_dirty)
		refresh ();

	ImU32 const accent = pal::imcol (pal::ACCENT);
	ImU32 const moss   = pal::imcol (pal::MOSS);

	// --- path bar ---
	ImGui::TextColored (ImGui::ColorConvertU32ToFloat4 (moss), "PATH");
	ImGui::SameLine ();
	ImGui::TextUnformatted (m_path.c_str ());

	// --- action bar ---
	bool const hasSel = (m_selected >= 0 && m_selected < static_cast<int> (m_entries.size ()));
	if (ImGui::SmallButton ("NEW FOLDER"))
	{
		m_nameBuf[0] = '\0';
		ImGui::OpenPopup ("mkdir");
	}
	if (hasSel)
	{
		ImGui::SameLine ();
		if (ImGui::SmallButton ("RENAME"))
		{
			std::snprintf (m_nameBuf, sizeof (m_nameBuf), "%s", m_entries[m_selected].name.c_str ());
			ImGui::OpenPopup ("rename");
		}
		ImGui::SameLine ();
		if (ImGui::SmallButton ("DELETE"))
			ImGui::OpenPopup ("delete?");
	}
	ImGui::Separator ();

	// --- popups ---
	if (ImGui::BeginPopup ("mkdir"))
	{
		ImGui::TextUnformatted ("New folder name:");
		ImGui::InputText ("##mkdir", m_nameBuf, sizeof (m_nameBuf), ImGuiInputTextFlags_AutoSelectAll);
		if (ImGui::Button ("CREATE") && m_nameBuf[0])
		{
			if (::mkdir (fullPath (m_nameBuf).c_str (), 0777) != 0)
				error ("mkdir failed: %s\n", m_nameBuf);
			else
				info ("created folder %s\n", m_nameBuf);
			m_dirty = true;
			ImGui::CloseCurrentPopup ();
		}
		ImGui::SameLine ();
		if (ImGui::Button ("CANCEL"))
			ImGui::CloseCurrentPopup ();
		ImGui::EndPopup ();
	}

	if (ImGui::BeginPopup ("rename"))
	{
		ImGui::TextUnformatted ("Rename to:");
		ImGui::InputText ("##rename", m_nameBuf, sizeof (m_nameBuf), ImGuiInputTextFlags_AutoSelectAll);
		if (ImGui::Button ("OK") && m_nameBuf[0] && hasSel)
		{
			auto const from = fullPath (m_entries[m_selected].name);
			auto const to   = fullPath (m_nameBuf);
			if (::rename (from.c_str (), to.c_str ()) != 0)
				error ("rename failed\n");
			else
				info ("renamed to %s\n", m_nameBuf);
			m_dirty = true;
			ImGui::CloseCurrentPopup ();
		}
		ImGui::SameLine ();
		if (ImGui::Button ("CANCEL"))
			ImGui::CloseCurrentPopup ();
		ImGui::EndPopup ();
	}

	if (ImGui::BeginPopup ("delete?"))
	{
		if (hasSel)
			ImGui::Text ("Delete %s?", m_entries[m_selected].name.c_str ());
		if (ImGui::Button ("YES, DELETE"))
		{
			doDelete ();
			ImGui::CloseCurrentPopup ();
		}
		ImGui::SameLine ();
		if (ImGui::Button ("CANCEL"))
			ImGui::CloseCurrentPopup ();
		ImGui::EndPopup ();
	}

	// --- list ---
	ImGui::BeginChild ("##files", ImVec2 (0, 0), false);

	if (m_path != ROOT)
	{
		if (ImGui::Selectable ("[..]", false))
			enter ("..");
	}

	for (int i = 0; i < static_cast<int> (m_entries.size ()); ++i)
	{
		auto const &e = m_entries[i];
		ImGui::PushID (i);

		if (e.isDir)
		{
			ImVec2 const p = ImGui::GetCursorScreenPos ();
			ImGui::GetWindowDrawList ()->AddRectFilled (
			    ImVec2 (p.x, p.y + 2), ImVec2 (p.x + 3, p.y + ImGui::GetTextLineHeight ()), accent);
			ImGui::Indent (8.0f);
		}

		char label[300];
		if (e.isDir)
			std::snprintf (label, sizeof (label), "%s/", e.name.c_str ());
		else
			std::snprintf (label,
			    sizeof (label),
			    "%-24.24s  %8s",
			    e.name.c_str (),
			    fs::printSize (e.size).c_str ());

		bool const isSel = (i == m_selected);
		if (ImGui::Selectable (label, isSel, ImGuiSelectableFlags_AllowDoubleClick))
		{
			if (e.isDir && ImGui::IsMouseDoubleClicked (0))
				enter (e.name); // double-tap a folder to open it
			else
				m_selected = i; // single tap selects
		}

		if (e.isDir)
			ImGui::Unindent (8.0f);

		ImGui::PopID ();
	}

	// stylus drag-to-scroll: pan the list by dragging, even over an item
	if (ImGui::IsWindowHovered (ImGuiHoveredFlags_ChildWindows |
	                            ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) &&
	    ImGui::IsMouseDragging (0, 4.0f))
		ImGui::SetScrollY (ImGui::GetScrollY () - ImGui::GetIO ().MouseDelta.y);

	ImGui::EndChild ();
}

} // namespace ph

#endif // CLASSIC
