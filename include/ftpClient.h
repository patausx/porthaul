// PORTHAUL — FTP client widget (top screen, CLIENT tab)
// Connects OUT to a remote FTP server (PC/phone/NAS) to LIST/DOWNLOAD/UPLOAD.
// All blocking libcurl work runs on a single worker thread; the UI thread only
// reads state guarded by a mutex. One operation at a time (rejects while BUSY).
#pragma once

#ifndef CLASSIC

#include "platform.h"

#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

namespace ph
{
/// \brief Simple threaded FTP client built on libcurl.
class FtpClient
{
public:
	/// \brief Connection / transfer status
	enum class Status
	{
		DISCONNECTED,
		CONNECTING,
		CONNECTED,
		BUSY,
		ERR,
	};

	/// \brief A remote directory entry
	struct Entry
	{
		std::string name;
		std::uint64_t size = 0;
		bool isDir         = false;
	};

	~FtpClient ();
	FtpClient ();

	FtpClient (FtpClient const &) = delete;
	FtpClient &operator= (FtpClient const &) = delete;

	/// \brief Store credentials and mark CONNECTING; caller should requestList().
	void connect (std::string host_, std::uint16_t port_, std::string user_, std::string pass_);

	/// \brief Drop credentials and clear state.
	void disconnect ();

	/// \brief Queue a directory listing of the current remote path.
	void requestList ();

	/// \brief Queue a download of remoteName_ into localPath_ (default sdmc:/<name>).
	void requestDownload (std::string const &remoteName_, std::string localPath_ = {});

	/// \brief Queue an upload of localPath_ to remoteName_ in the current path.
	void requestUpload (std::string const &localPath_, std::string const &remoteName_);

	/// \brief Change into a remote subdirectory, then list.
	void cd (std::string const &name_);

	/// \brief Go up one remote directory, then list.
	void cdUp ();

	// --- UI-thread accessors (snapshot under mutex) ---

	/// \brief Current status
	Status status ();

	/// \brief Current remote path (e.g. "/sub/")
	std::string path ();

	/// \brief Last error message (valid when status()==ERR)
	std::string error ();

	/// \brief Copy of remote entries (dirs first)
	std::vector<Entry> entries ();

	/// \brief Transfer progress 0..1 (0 when unknown)
	float progress ();

private:
	/// \brief Pending operation kind
	enum class Op
	{
		NONE,
		LIST,
		DOWNLOAD,
		UPLOAD,
	};

	/// \brief Worker thread entry point
	void worker ();

	/// \brief Build "ftp://host:port/" prefix (path appended by caller)
	std::string urlBase ();

	/// \brief Run blocking listing of m_path; fills m_entries
	void doList ();

	/// \brief Run blocking download of m_opName -> m_opLocal
	void doDownload ();

	/// \brief Run blocking upload of m_opLocal -> m_opName
	void doUpload ();

	/// \brief Parse a unix `ls -l` style listing into m_entries
	void parseListing (std::string const &raw_);

	platform::Mutex m_lock;
	platform::Thread m_thread;
	std::atomic_bool m_quit = false;

	// --- shared state (guard with m_lock) ---
	Status m_status = Status::DISCONNECTED;
	std::string m_host;
	std::uint16_t m_port = 21;
	std::string m_user;
	std::string m_pass;
	std::string m_path = "/";
	std::string m_error;
	std::vector<Entry> m_entries;

	// pending op (guard with m_lock)
	Op m_op = Op::NONE;
	std::string m_opName;  // remote name
	std::string m_opLocal; // local path

	// progress, written by curl callback on worker thread
	std::atomic<float> m_progress = 0.0f;
};

} // namespace ph

#endif // CLASSIC
