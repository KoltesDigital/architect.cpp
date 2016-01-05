#pragma once

#include <list>
#include <map>
#include <set>
#include <json.hpp>
#include <architect/Symbol.hpp>

namespace architect
{
	struct ComputeCyclesParameters
	{
		uint32_t minCardinality;

		ComputeCyclesParameters()
			: minCardinality(0)
		{}
	};

	class Registry
	{
	public:
		Namespace rootNameSpace;

		Registry();
		~Registry();

		void clear();

		Namespace *createNamespace();
		Symbol *createSymbol(SymbolType type, bool defined);

		const std::map<SymbolId, Symbol *> &getSymbols() const;

		void removeRedundantDependencies();
		std::set<std::list<const Symbol *>> computeCycles(const ComputeCyclesParameters &parameters = ComputeCyclesParameters()) const;
		std::set<std::list<const Symbol *>> computeScc(const ComputeCyclesParameters &parameters = ComputeCyclesParameters()) const;

		bool operator==(const Registry &other) const;

	private:
		std::set<Namespace *> _namespaces;
		std::map<SymbolId, Symbol *> _symbols;

		SymbolId _nextSymbolId;
	};
}
