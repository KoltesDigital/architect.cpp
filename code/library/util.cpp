#include <architect/util.hpp>

namespace architect
{
	namespace util
	{
		bool absolutePath(std::string &path)
		{
#ifdef _WIN32
			char buffer[MAX_PATH];

			auto retval = GetFullPathNameA(path.c_str(), MAX_PATH, buffer, nullptr);
			if (!retval)
				return false;

			path.assign(buffer, retval);
			return true;
#else
			char buffer[PATH_MAX + 1];

			auto retval = realpath(path.c_str(), buffer);
			if (!retval)
				return false;

			path = buffer;
			return true;
#endif
		}

		bool currentWorkingDirectory(std::string &path)
		{
#ifdef _WIN32
			char buffer[MAX_PATH];
			auto retval = _getcwd(buffer, MAX_PATH);
#else
			char buffer[PATH_MAX + 1];
			auto retval = getcwd(buffer, PATH_MAX + 1);
#endif
			if (!retval)
				return false;

			path = buffer;
			return true;
		}
	}
}