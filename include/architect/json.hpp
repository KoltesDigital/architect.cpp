#pragma once
#ifdef ARCHITECT_JSON_SUPPORT

#include <istream>
#include <list>
#include <map>
#include <ostream>
#include <set>
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

		void dumpCycles(const std::set<std::list<const Symbol *>> &cycles, nlohmann::json &j, const FormattingParameters &parameters = FormattingParameters());
		void dumpCycles(const std::set<std::list<const Symbol *>> &cycles, std::ostream &stream, const FormattingParameters &parameters = FormattingParameters());

		void dumpSymbols(const std::map<SymbolId, Symbol *> &symbols, nlohmann::json &j, const FormattingParameters &parameters = FormattingParameters());
		void dumpSymbols(const std::map<SymbolId, Symbol *> &symbols, std::ostream &stream, const FormattingParameters &parameters = FormattingParameters());
	}
}

#endif
