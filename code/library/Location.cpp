#include <architect/Location.hpp>

namespace architect
{
#ifdef ARCHITECT_CLANG_SUPPORT
	void Location::getFromCursor(const CXCursor &cursor)
	{
		CXString cfilename;
		CXSourceLocation sourceLocation = clang_getCursorLocation(cursor);
		clang_getPresumedLocation(sourceLocation, &cfilename, &line, &column);
		filename = clang_getCString(cfilename);
	}
#endif

	bool Location::operator<(const Location &other) const
	{
		int compare = filename.compare(other.filename);
		if (compare)
			return compare < 0;

		if (line != other.line)
			return line < other.line;

		if (column != other.column)
			return column < other.column;

		return false;
	}
}
