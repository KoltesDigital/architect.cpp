#pragma once

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <cstdlib>
#include <unistd.h>
#endif

#include <string>

namespace architect
{
	namespace util
	{
		bool absolutePath(std::string &path);
		bool currentWorkingDirectory(std::string &path);
	}
}
