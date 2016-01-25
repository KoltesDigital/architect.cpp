#pragma once
#ifdef ARCHITECT_DOT_SUPPORT

#include <ostream>
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

		void dumpCycles(const Cycles &cycles, std::ostream &stream, const FormattingParameters &parameters = FormattingParameters());
		
		void dumpSymbols(const Symbols &symbols, std::ostream &stream, const FormattingParameters &parameters = FormattingParameters());
	}
}

#endif
