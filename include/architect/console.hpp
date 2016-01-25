#pragma once
#ifdef ARCHITECT_CONSOLE_SUPPORT

#include <ostream>
#include <architect/Symbol.hpp>

namespace architect
{
	class Registry;

	namespace console
	{
		void dumpCycles(const Cycles &cycles, std::ostream &stream);

		void dumpSymbols(const Symbols &symbols, std::ostream &stream);
	}
}

#endif
