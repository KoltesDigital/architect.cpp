#ifdef ARCHITECT_CONSOLE_SUPPORT
#include <architect/console.hpp>

#include <architect/Registry.hpp>

namespace architect
{
	namespace
	{
		const char *getReferenceTypeName(ReferenceType type)
		{
			switch (type)
			{
			case ReferenceType::TEMPLATE:
				return "template specialization";
			case ReferenceType::INHERITANCE:
				return "inheritance";
			case ReferenceType::COMPOSITION:
				return "composition";
			case ReferenceType::ASSOCIATION:
				return "association";
			default:
				return "???";
			}
		}

		const char *getSymbolTypeName(SymbolType type)
		{
			switch (type)
			{
			case SymbolType::GLOBAL:
				return "global";
			case SymbolType::GLOBAL_TEMPLATE:
				return "global template";
			case SymbolType::RECORD:
				return "record";
			case SymbolType::RECORD_TEMPLATE:
				return "record template";
			case SymbolType::TYPEDEF:
				return "typedef";
			default:
				return "???";
			}
		}
	}

	namespace console
	{
		void dumpCycles(const Cycles &cycles, std::ostream &stream)
		{
			for (auto &cycle : cycles)
			{
				stream << "- ";
				for (auto symbol : cycle)
				{
					stream << symbol->getFullName() << " -> ";
				}

				stream << (*cycle.begin())->getFullName() << "\n";
			}
		}

		void dumpSymbols(const Symbols &symbols, std::ostream &stream)
		{
			for (auto &pair : symbols)
			{
				const Symbol *symbol = pair.second;

				stream << symbol->getFullName() << " (" << getSymbolTypeName(symbol->type) << ")\n";

				for (auto &refPair : symbol->references)
				{
					stream << "  " << symbols.at(refPair.first)->getFullName() << "\n";

					for (auto &reference : refPair.second)
					{
						stream << "    " << reference.location.filename << ":" << reference.location.line << "," << reference.location.column << " (" << getReferenceTypeName(reference.type) << ")\n";
					}
				}
				stream << "\n";
			}
		}
	}
}

#endif
