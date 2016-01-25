#pragma once

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

		const Symbols &getSymbols() const;

		void removeRedundantDependencies();
		Cycles computeCycles(const ComputeCyclesParameters &parameters = ComputeCyclesParameters()) const;
		Cycles computeScc(const ComputeCyclesParameters &parameters = ComputeCyclesParameters()) const;

		bool operator==(const Registry &other) const;

	private:
		std::set<Namespace *> _namespaces;
		Symbols _symbols;

		SymbolId _nextSymbolId;
	};
}
