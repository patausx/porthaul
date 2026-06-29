// PORTHAUL — local file browser widget (bottom screen)
// Self-contained: holds a current directory, lists entries via POSIX dirent,
// renders a touch list through Dear ImGui. Tap a row to select; an action bar
// offers DELETE / RENAME / NEW FOLDER. Brutalist styling via ph_palette.
#pragma once

#ifndef CLASSIC

#include <cstdint>
#include <string>
#include <vector>

namespace ph {

class FileBrowser
{
public:
	struct Entry
	{
		std::string name;
		std::uint64_t size = 0;
		bool isDir         = false;
	};

	FileBrowser ();

	/// \brief Draw the browser in the current ImGui window. Call inside a window.
	void draw ();

	/// \brief Current directory path
	std::string const &path () const { return m_path; }

	/// \brief Force a re-scan on next draw (e.g. after an upload finished)
	void markDirty () { m_dirty = true; }

	/// \brief Full path of the currently-selected FILE (empty if none or a dir).
	/// Used by the client UI to upload the picked local file.
	std::string selectedFile () const
	{
		if (m_selected < 0 || m_selected >= static_cast<int> (m_entries.size ()))
			return {};
		if (m_entries[m_selected].isDir)
			return {};
		return fullPath (m_entries[m_selected].name);
	}

	/// \brief Bare name of the selected file (empty if none/dir)
	std::string selectedName () const
	{
		if (m_selected < 0 || m_selected >= static_cast<int> (m_entries.size ()))
			return {};
		if (m_entries[m_selected].isDir)
			return {};
		return m_entries[m_selected].name;
	}

private:
	void refresh ();
	void enter (std::string const &name_);

	/// \brief Build an absolute path for a name in the current dir
	std::string fullPath (std::string const &name_) const;

	/// \brief Perform a delete of the selected entry
	void doDelete ();

	std::string m_path;
	std::vector<Entry> m_entries;
	bool m_dirty = true;

	int m_selected = -1;          // index into m_entries, -1 = none

	// rename / mkdir name input (soft keyboard fills this via InputText)
	char m_nameBuf[256] = {0};
};

} // namespace ph

#endif // CLASSIC
