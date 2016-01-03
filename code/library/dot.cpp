#ifdef ARCHITECT_DOT_SUPPORT
#include <architect/dot.hpp>

#include <string>
#include <architect/Registry.hpp>

namespace architect
{
	namespace dot
	{
		namespace
		{
			class Attributes
			{
			public:
				template <typename T>
				void insert(std::string key, T value)
				{
					insert(key, std::to_string(value));
				}

				template <>
				void insert(std::string key, const char *value)
				{
					insert(key, std::string(value));
				}

				template <>
				void insert(std::string key, std::string value)
				{
					_map.insert(std::pair<std::string, std::string>(key, value));
				}

			private:
				std::map<std::string, std::string> _map;
				friend std::ostream &operator<<(std::ostream &stream, const Attributes &attributes);
			};

			std::ostream &operator<<(std::ostream &stream, const Attributes &attributes)
			{
				if (!attributes._map.empty())
				{
					stream << "[";
					for (auto &pair : attributes._map)
					{
						stream << pair.first << "=" << pair.second << ";";
					}
					stream << "]";
				}
				return stream;
			}

			void outputSymbol(const Symbol *symbol, std::ostream &stream, const FormattingParameters &parameters)
			{
				Attributes attributes;

				attributes.insert("label", "\"" + symbol->getFullName() + "\"");

				switch (symbol->type)
				{
				case SymbolType::GLOBAL:
					attributes.insert("shape", "ellipse");
					break;

				case SymbolType::RECORD:
					attributes.insert("shape", "box");
					break;

				case SymbolType::TYPEDEF:
					attributes.insert("shape", "octagon");
					break;
				}

				if (parameters.pretty)
					stream << "  ";
				stream << symbol->id << attributes << ";";
				if (parameters.pretty)
					stream << "\n";
			}
		}

		FormattingParameters::FormattingParameters()
			: displayReferenceCount(false)
			, pretty(false)
		{}

		void dumpCycles(const std::set<std::list<const Symbol *>> &cycles, std::ostream &stream, const FormattingParameters &parameters)
		{
			std::set<const Symbol *> visitedSymbols;

			stream << "strict digraph{";
			if (parameters.pretty)
				stream << "\n";

			for (auto &cycle : cycles)
			{
				for (auto symbol : cycle)
				{
					if (visitedSymbols.find(symbol) == visitedSymbols.end())
					{
						visitedSymbols.insert(symbol);
						outputSymbol(symbol, stream, parameters);
					}
				}
			}

			for (auto &cycle : cycles)
			{
				SymbolId previousId = cycle.back()->id;

				for (auto symbol : cycle)
				{
					if (parameters.pretty)
						stream << "  ";
					stream << previousId << "->" << symbol->id << ";";
					if (parameters.pretty)
						stream << "\n";
					
					previousId = symbol->id;
				}
			}

			stream << "}\n";
		}

		void dumpSymbols(const std::map<SymbolId, Symbol *> &symbols, std::ostream &stream, const FormattingParameters &parameters)
		{
			stream << "strict digraph{";

			for (auto &pair : symbols)
			{
				const Symbol *symbol = pair.second;
				outputSymbol(symbol, stream, parameters);
			}

			for (auto &pair : symbols)
			{
				const Symbol *parent = pair.second;

				for (auto &pair : parent->references)
				{
					Symbol *child = symbols.at(pair.first);
					auto &referenceSet = pair.second;
					auto &mostImportantDep = *referenceSet.begin();

					Attributes attributes;

					if (parameters.displayReferenceCount)
						attributes.insert("label", referenceSet.size());

					switch (mostImportantDep.type)
					{
					case ReferenceType::TEMPLATE:
						attributes.insert("arrowtail", "invempty");
						attributes.insert("dir", "both");
						break;

					case ReferenceType::INHERITANCE:
						attributes.insert("arrowhead", "empty");
						break;

					case ReferenceType::COMPOSITION:
						attributes.insert("arrowtail", "diamond");
						attributes.insert("dir", "both");
						break;
					}

					if (parameters.pretty)
						stream << "  ";
					stream << parent->id << "->" << child->id << attributes << ";";
					if (parameters.pretty)
						stream << "\n";
				}
			}

			stream << "}\n";
		}
	}
}

#endif
