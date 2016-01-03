#pragma once
#ifdef ARCHITECT_CONSOLE_SUPPORT

#include <list>
#include <map>
#include <ostream>
#include <set>
#include <architect/Symbol.hpp>

namespace architect
{
	class Registry;

	namespace console
	{
		void dumpCycles(const std::set<std::list<const Symbol *>> &cycles, std::ostream &stream);

		void dumpSymbols(const std::map<SymbolId, Symbol *> &symbols, std::ostream &stream);
	}
}

#endif
