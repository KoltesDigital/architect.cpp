#pragma once
#ifdef ARCHITECT_JSON_SUPPORT

#include <istream>
#include <ostream>
#include <json.hpp>
#include <architect/Symbol.hpp>

namespace architect
{
	class Registry;

	namespace json
	{
		struct FormattingParameters
		{
			bool pretty; // whether to format with indentations and line returns, only for ostream

			FormattingParameters();
		};

		bool parse(Registry &registry, const nlohmann::json &j);
		bool parse(Registry &registry, std::istream &stream);

		void dumpCycles(const Cycles &cycles, nlohmann::json &j, const FormattingParameters &parameters = FormattingParameters());
		void dumpCycles(const Cycles &cycles, std::ostream &stream, const FormattingParameters &parameters = FormattingParameters());

		void dumpSymbols(const Symbols &symbols, nlohmann::json &j, const FormattingParameters &parameters = FormattingParameters());
		void dumpSymbols(const Symbols &symbols, std::ostream &stream, const FormattingParameters &parameters = FormattingParameters());
	}
}

#endif
