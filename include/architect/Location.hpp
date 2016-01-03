#pragma once

#include <string>

#ifdef ARCHITECT_CLANG_SUPPORT
#include <clang-c/Index.h>
#endif

namespace architect
{
	class Location
	{
	public:
#ifdef ARCHITECT_CLANG_SUPPORT
		void getFromCursor(const CXCursor &cursor);
#endif

		bool operator<(const Location &other) const;

		std::string filename;
		uint32_t line, column;
	};
}
