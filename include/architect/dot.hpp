#pragma once
#ifdef ARCHITECT_DOT_SUPPORT

#include <list>
#include <map>
#include <ostream>
#include <set>
#include <architect/Symbol.hpp>

namespace architect
{
	struct ComputeCyclesParameters;
	class Registry;

	namespace dot
	{
		struct FormattingParameters
		{
			FormattingParameters();

			bool displayReferenceCount; // whether to display the number of references on edges
			bool pretty; // whether to format with indentations and line returns
		};

		void dumpCycles(const std::set<std::list<const Symbol *>> &cycles, std::ostream &stream, const FormattingParameters &parameters = FormattingParameters());
		
		void dumpSymbols(const std::map<SymbolId, Symbol *> &symbols, std::ostream &stream, const FormattingParameters &parameters = FormattingParameters());
	}
}

#endif
