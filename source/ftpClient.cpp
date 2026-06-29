// PORTHAUL — FTP client widget. See ftpClient.h.
#ifndef CLASSIC

#include "ftpClient.h"

#include "log.h"

#include <curl/curl.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <mutex>
#include <sstream>

namespace ph
{
namespace
{
#if defined(__3DS__) || defined(__SWITCH__)
constexpr char const *LOCAL_ROOT = "sdmc:/";
#else
constexpr char const *LOCAL_ROOT = "/tmp/";
#endif

/// \brief curl write callback — append to a std::string
std::size_t writeToString (char *ptr_, std::size_t size_, std::size_t nmemb_, void *user_)
{
	auto *const str   = static_cast<std::string *> (user_);
	std::size_t const n = size_ * nmemb_;
	str->append (ptr_, n);
	return n;
}

/// \brief curl write callback — write to a FILE*
std::size_t writeToFile (char *ptr_, std::size_t size_, std::size_t nmemb_, void *user_)
{
	auto *const fp = static_cast<std::FILE *> (user_);
	return std::fwrite (ptr_, size_, nmemb_, fp);
}

/// \brief curl read callback — read from a FILE*
std::size_t readFromFile (char *ptr_, std::size_t size_, std::size_t nmemb_, void *user_)
{
	auto *const fp = static_cast<std::FILE *> (user_);
	return std::fread (ptr_, size_, nmemb_, fp);
}

/// \brief curl progress callback — store fraction in an atomic<float>
int xferInfo (void *user_,
    curl_off_t dlTotal_,
    curl_off_t dlNow_,
    curl_off_t ulTotal_,
    curl_off_t ulNow_)
{
	auto *const prog = static_cast<std::atomic<float> *> (user_);
	curl_off_t const total = dlTotal_ > 0 ? dlTotal_ : ulTotal_;
	curl_off_t const now   = dlTotal_ > 0 ? dlNow_ : ulNow_;
	if (total > 0)
		prog->store (static_cast<float> (now) / static_cast<float> (total));
	return 0;
}
} // namespace

FtpClient::FtpClient () : m_thread ([this] () { worker (); })
{
}

FtpClient::~FtpClient ()
{
	m_quit = true;
	m_thread.join ();
}

void FtpClient::connect (
    std::string host_, std::uint16_t port_, std::string user_, std::string pass_)
{
	auto const lock = std::scoped_lock (m_lock);
	m_host          = std::move (host_);
	m_port          = port_;
	m_user          = std::move (user_);
	m_pass          = std::move (pass_);
	m_path          = "/";
	m_entries.clear ();
	m_error.clear ();
	m_status = Status::CONNECTING;
}

void FtpClient::disconnect ()
{
	auto const lock = std::scoped_lock (m_lock);
	m_status        = Status::DISCONNECTED;
	m_op            = Op::NONE;
	m_entries.clear ();
	m_error.clear ();
	m_path = "/";
}

void FtpClient::requestList ()
{
	auto const lock = std::scoped_lock (m_lock);
	if (m_status == Status::BUSY || m_op != Op::NONE)
		return;
	m_op     = Op::LIST;
	m_status = Status::BUSY;
	m_progress.store (0.0f);
}

void FtpClient::requestDownload (std::string const &remoteName_, std::string localPath_)
{
	auto const lock = std::scoped_lock (m_lock);
	if (m_status == Status::BUSY || m_op != Op::NONE)
		return;
	if (localPath_.empty ())
		localPath_ = std::string (LOCAL_ROOT) + remoteName_;
	m_op      = Op::DOWNLOAD;
	m_opName  = remoteName_;
	m_opLocal = std::move (localPath_);
	m_status  = Status::BUSY;
	m_progress.store (0.0f);
}

void FtpClient::requestUpload (std::string const &localPath_, std::string const &remoteName_)
{
	auto const lock = std::scoped_lock (m_lock);
	if (m_status == Status::BUSY || m_op != Op::NONE)
		return;
	m_op      = Op::UPLOAD;
	m_opName  = remoteName_;
	m_opLocal = localPath_;
	m_status  = Status::BUSY;
	m_progress.store (0.0f);
}

void FtpClient::cd (std::string const &name_)
{
	{
		auto const lock = std::scoped_lock (m_lock);
		if (m_status == Status::BUSY || m_op != Op::NONE)
			return;
		if (!m_path.empty () && m_path.back () != '/')
			m_path += '/';
		m_path += name_;
		m_path += '/';
	}
	requestList ();
}

void FtpClient::cdUp ()
{
	{
		auto const lock = std::scoped_lock (m_lock);
		if (m_status == Status::BUSY || m_op != Op::NONE)
			return;
		std::string p = m_path;
		if (p.size () > 1 && p.back () == '/')
			p.pop_back ();
		auto const slash = p.find_last_of ('/');
		if (slash != std::string::npos)
			p = p.substr (0, slash + 1);
		else
			p = "/";
		if (p.empty ())
			p = "/";
		m_path = p;
	}
	requestList ();
}

FtpClient::Status FtpClient::status ()
{
	auto const lock = std::scoped_lock (m_lock);
	return m_status;
}

std::string FtpClient::path ()
{
	auto const lock = std::scoped_lock (m_lock);
	return m_path;
}

std::string FtpClient::error ()
{
	auto const lock = std::scoped_lock (m_lock);
	return m_error;
}

std::vector<FtpClient::Entry> FtpClient::entries ()
{
	auto const lock = std::scoped_lock (m_lock);
	return m_entries;
}

float FtpClient::progress ()
{
	return m_progress.load ();
}

std::string FtpClient::urlBase ()
{
	// caller holds the lock
	char buf[256];
	std::snprintf (buf, sizeof (buf), "ftp://%s:%u", m_host.c_str (), m_port);
	return buf;
}

void FtpClient::worker ()
{
	while (!m_quit.load ())
	{
		Op op = Op::NONE;
		{
			auto const lock = std::scoped_lock (m_lock);
			op              = m_op;
		}

		if (op == Op::NONE)
		{
			platform::Thread::sleep (std::chrono::milliseconds (50));
			continue;
		}

		switch (op)
		{
		case Op::LIST:
			doList ();
			break;
		case Op::DOWNLOAD:
			doDownload ();
			break;
		case Op::UPLOAD:
			doUpload ();
			break;
		case Op::NONE:
			break;
		}

		auto const lock = std::scoped_lock (m_lock);
		m_op = Op::NONE;
	}
}

void FtpClient::parseListing (std::string const &raw_)
{
	// caller holds the lock
	m_entries.clear ();

	std::istringstream stream (raw_);
	std::string line;
	while (std::getline (stream, line))
	{
		if (!line.empty () && line.back () == '\r')
			line.pop_back ();
		if (line.empty ())
			continue;

		// tolerant unix `ls -l` parse: split on whitespace
		std::vector<std::string> fields;
		std::istringstream ls (line);
		std::string tok;
		while (ls >> tok)
			fields.push_back (tok);

		Entry e;
		if (fields.size () >= 9 && (line[0] == 'd' || line[0] == '-' || line[0] == 'l'))
		{
			bool const isDir = (line[0] == 'd');
			std::uint64_t size = 0;
			std::sscanf (fields[4].c_str (), "%llu", reinterpret_cast<unsigned long long *> (&size));

			// name = everything from field 9 onward (rejoined with spaces)
			std::string name = fields[8];
			for (std::size_t i = 9; i < fields.size (); ++i)
			{
				name += ' ';
				name += fields[i];
			}
			// symlink "name -> target": keep just the link name
			if (line[0] == 'l')
			{
				auto const arrow = name.find (" -> ");
				if (arrow != std::string::npos)
					name = name.substr (0, arrow);
			}

			if (name == "." || name == "..")
				continue;

			e.name  = name;
			e.size  = size;
			e.isDir = isDir;
		}
		else
		{
			// fallback: treat the whole (trimmed) line as a bare name
			auto const first = line.find_first_not_of (" \t");
			if (first == std::string::npos)
				continue;
			std::string name = line.substr (first);
			if (name == "." || name == "..")
				continue;
			e.name  = name;
			e.isDir = false;
		}
		m_entries.push_back (std::move (e));
	}

	std::sort (m_entries.begin (), m_entries.end (), [] (Entry const &a, Entry const &b) {
		if (a.isDir != b.isDir)
			return a.isDir > b.isDir;
		return std::lexicographical_compare (a.name.begin (),
		    a.name.end (),
		    b.name.begin (),
		    b.name.end (),
		    [] (char x, char y) {
			    return std::tolower (static_cast<unsigned char> (x)) <
			           std::tolower (static_cast<unsigned char> (y));
		    });
	});
}

void FtpClient::doList ()
{
	std::string url;
	std::string userpwd;
	{
		auto const lock = std::scoped_lock (m_lock);
		url             = urlBase ();
		if (!m_path.empty () && m_path.front () != '/')
			url += '/';
		url += m_path;
		if (url.empty () || url.back () != '/')
			url += '/';
		userpwd = m_user + ":" + m_pass;
	}

	CURL *const curl = curl_easy_init ();
	if (!curl)
	{
		auto const lock = std::scoped_lock (m_lock);
		m_error         = "curl init failed";
		m_status        = Status::ERR;
		return;
	}

	std::string body;
	curl_easy_setopt (curl, CURLOPT_URL, url.c_str ());
	curl_easy_setopt (curl, CURLOPT_USERPWD, userpwd.c_str ());
	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, writeToString);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, &body);
	curl_easy_setopt (curl, CURLOPT_DIRLISTONLY, 0L);
	curl_easy_setopt (curl, CURLOPT_FTP_USE_EPSV, 0L);
	curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT, 10L);
	curl_easy_setopt (curl, CURLOPT_TIMEOUT, 30L);

	CURLcode const rc = curl_easy_perform (curl);
	curl_easy_cleanup (curl);

	auto const lock = std::scoped_lock (m_lock);
	if (rc != CURLE_OK)
	{
		m_error  = curl_easy_strerror (rc);
		m_status = Status::ERR;
		::error ("ftp list failed: %s\n", m_error.c_str ());
		return;
	}

	parseListing (body);
	m_status = Status::CONNECTED;
	info ("ftp listed %zu entries\n", m_entries.size ());
}

void FtpClient::doDownload ()
{
	std::string url;
	std::string userpwd;
	std::string local;
	{
		auto const lock = std::scoped_lock (m_lock);
		url             = urlBase ();
		if (!m_path.empty () && m_path.front () != '/')
			url += '/';
		url += m_path;
		if (url.empty () || url.back () != '/')
			url += '/';
		url += m_opName;
		userpwd = m_user + ":" + m_pass;
		local   = m_opLocal;
	}

	std::FILE *const fp = std::fopen (local.c_str (), "wb");
	if (!fp)
	{
		auto const lock = std::scoped_lock (m_lock);
		m_error         = "cannot open local file";
		m_status        = Status::ERR;
		::error ("download: cannot open %s\n", local.c_str ());
		return;
	}

	CURL *const curl = curl_easy_init ();
	if (!curl)
	{
		std::fclose (fp);
		auto const lock = std::scoped_lock (m_lock);
		m_error         = "curl init failed";
		m_status        = Status::ERR;
		return;
	}

	curl_easy_setopt (curl, CURLOPT_URL, url.c_str ());
	curl_easy_setopt (curl, CURLOPT_USERPWD, userpwd.c_str ());
	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, writeToFile);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt (curl, CURLOPT_FTP_USE_EPSV, 0L);
	curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT, 10L);
	curl_easy_setopt (curl, CURLOPT_TIMEOUT, 0L);
	curl_easy_setopt (curl, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt (curl, CURLOPT_XFERINFOFUNCTION, xferInfo);
	curl_easy_setopt (curl, CURLOPT_XFERINFODATA, &m_progress);

	CURLcode const rc = curl_easy_perform (curl);
	curl_easy_cleanup (curl);
	std::fclose (fp);

	auto const lock = std::scoped_lock (m_lock);
	if (rc != CURLE_OK)
	{
		m_error  = curl_easy_strerror (rc);
		m_status = Status::ERR;
		::error ("download failed: %s\n", m_error.c_str ());
		return;
	}
	m_status = Status::CONNECTED;
	info ("downloaded %s -> %s\n", m_opName.c_str (), local.c_str ());
}

void FtpClient::doUpload ()
{
	std::string url;
	std::string userpwd;
	std::string local;
	{
		auto const lock = std::scoped_lock (m_lock);
		url             = urlBase ();
		if (!m_path.empty () && m_path.front () != '/')
			url += '/';
		url += m_path;
		if (url.empty () || url.back () != '/')
			url += '/';
		url += m_opName;
		userpwd = m_user + ":" + m_pass;
		local   = m_opLocal;
	}

	std::FILE *const fp = std::fopen (local.c_str (), "rb");
	if (!fp)
	{
		auto const lock = std::scoped_lock (m_lock);
		m_error         = "cannot open local file";
		m_status        = Status::ERR;
		::error ("upload: cannot open %s\n", local.c_str ());
		return;
	}

	// determine size for INFILESIZE
	std::fseek (fp, 0, SEEK_END);
	long const fileSize = std::ftell (fp);
	std::fseek (fp, 0, SEEK_SET);

	CURL *const curl = curl_easy_init ();
	if (!curl)
	{
		std::fclose (fp);
		auto const lock = std::scoped_lock (m_lock);
		m_error         = "curl init failed";
		m_status        = Status::ERR;
		return;
	}

	curl_easy_setopt (curl, CURLOPT_URL, url.c_str ());
	curl_easy_setopt (curl, CURLOPT_USERPWD, userpwd.c_str ());
	curl_easy_setopt (curl, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt (curl, CURLOPT_READFUNCTION, readFromFile);
	curl_easy_setopt (curl, CURLOPT_READDATA, fp);
	curl_easy_setopt (
	    curl, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t> (fileSize));
	curl_easy_setopt (curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);
	curl_easy_setopt (curl, CURLOPT_FTP_USE_EPSV, 0L);
	curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT, 10L);
	curl_easy_setopt (curl, CURLOPT_TIMEOUT, 0L);
	curl_easy_setopt (curl, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt (curl, CURLOPT_XFERINFOFUNCTION, xferInfo);
	curl_easy_setopt (curl, CURLOPT_XFERINFODATA, &m_progress);

	CURLcode const rc = curl_easy_perform (curl);
	curl_easy_cleanup (curl);
	std::fclose (fp);

	auto const lock = std::scoped_lock (m_lock);
	if (rc != CURLE_OK)
	{
		m_error  = curl_easy_strerror (rc);
		m_status = Status::ERR;
		::error ("upload failed: %s\n", m_error.c_str ());
		return;
	}
	m_status = Status::CONNECTED;
	info ("uploaded %s -> %s\n", local.c_str (), m_opName.c_str ());
}

} // namespace ph

#endif // CLASSIC
