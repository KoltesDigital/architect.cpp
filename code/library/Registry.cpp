#include <architect/Registry.hpp>

#include <algorithm>
#include <stack>
#include <architect/Symbol.hpp>

namespace architect
{
	namespace
	{
		struct SccData
		{
			uint32_t index;
			uint32_t lowlink;
			bool onStack;
		};

		struct SccContext : ComputeCyclesParameters
		{
			const Symbols &symbols;
			std::map<SymbolId, SccData> meta;
			std::stack<const Symbol *> stack;
			Cycles cluters;
			uint32_t index;

			SccContext(const ComputeCyclesParameters &parameters, const Symbols &_symbols)
				: ComputeCyclesParameters(parameters)
				, symbols(_symbols)
				, index(0)
			{}
		};

		uint32_t strongConnect(const Symbol *symbol, SccContext &context)
		{
			SccData &data = context.meta[symbol->id];
			data.index = context.index;
			data.lowlink = context.index;
			data.onStack = true;
			++context.index;

			context.stack.push(symbol);

			for (auto &pair : symbol->references)
			{
				auto refId = pair.first;
				auto it = context.meta.find(refId);
				if (it == context.meta.end())
				{
					Symbol *refSymbol = context.symbols.at(refId);
					auto lowlink = strongConnect(refSymbol, context);
					data.lowlink = std::min(data.lowlink, lowlink);
				}
				else
				{
					auto &refData = it->second;
					if (refData.onStack)
					{
						data.lowlink = std::min(data.lowlink, refData.index);
					}
				}
			}

			if (data.lowlink == data.index)
			{
				std::list<const Symbol *> cycle;
				const Symbol *cycleSymbol;
				do
				{
					cycleSymbol = context.stack.top();
					context.stack.pop();

					auto it = context.meta.find(cycleSymbol->id);
					if (it != context.meta.end())
					{
						auto &pair = *it;
						pair.second.onStack = false;

						cycle.push_back(cycleSymbol);
					}
				} while (cycleSymbol != symbol);

				if ((uint32_t)cycle.size() > context.minCardinality)
				context.cluters.insert(cycle);
			}

			return data.lowlink;
		}
	}

	Registry::Registry()
	{
		clear();
	}

	Registry::~Registry()
	{
		clear();
	}

	void Registry::clear()
	{
		for (auto ns : _namespaces)
			delete ns;
		_namespaces.clear();

		for (auto pair : _symbols)
			delete pair.second;
		_symbols.clear();
		
		rootNameSpace = Namespace();
		rootNameSpace.parent = nullptr;
		
		_nextSymbolId = 0;
	}

	Namespace *Registry::createNamespace()
	{
		auto ns = new Namespace();
		_namespaces.insert(ns);
		return ns;
	}

	Symbol *Registry::createSymbol(SymbolType type, bool defined)
	{
		auto symbol = new Symbol();
		symbol->id = _nextSymbolId;
		symbol->type = type;
		symbol->defined = defined;
		_symbols.insert(std::pair<SymbolId, Symbol *>(_nextSymbolId, symbol));
		++_nextSymbolId;
		return symbol;
	}

	const Symbols &Registry::getSymbols() const
	{
		return _symbols;
	}

	void Registry::removeRedundantDependencies()
	{
		// Root -> A, C
		// A -> B
		// B -> C
		// then remove Root -> C

		for (auto &pair : _symbols)
		{
			std::list<SymbolId> references;

			auto &symbolRoot = pair.second;
			auto &dependenciesRoot = symbolRoot->references;

			for (auto &dependency : dependenciesRoot)
			{
				references.push_back(dependency.first);
			}

			for (auto &dependency : references)
			{
				auto &subSymbol = _symbols[dependency];
				for (auto &subDependency : subSymbol->references)
				{
					references.push_back(subDependency.first);
				}
			}

			for (auto &a : references)
			{
				auto &symbolA = _symbols[a];
				auto &dependenciesA = symbolA->references;

				auto itB = dependenciesRoot.begin();
				while (itB != dependenciesRoot.end())
				{
					auto &symbolB = itB->first;
					if (dependenciesA.find(symbolB) != dependenciesA.end())
					{
						itB = dependenciesRoot.erase(itB);
					}
					else
					{
						++itB;
					}
				}
			}
		}
	}

	Cycles Registry::computeCycles(const ComputeCyclesParameters &parameters) const
	{
		Cycles cycles;
		std::set<const Symbol *> visitedSymbols;

		for (auto &pair : _symbols)
		{
			auto symbol = pair.second;
			visitedSymbols.insert(symbol);

			std::list<std::list<const Symbol *>> branches;

			std::list<const Symbol *> branch;
			branch.push_back(symbol);
			branches.push_back(branch);

			while (branches.size())
			{
				auto &branch = branches.front();

				const Symbol *parent = branch.back();
				for (auto &pair : parent->references)
				{
					auto child = _symbols.at(pair.first);

					if (child == symbol)
					{
						if ((uint32_t)branch.size() > parameters.minCardinality)
							cycles.insert(branch);
						break;
					}

					if (visitedSymbols.find(child) != visitedSymbols.end())
						continue;

					auto it = branch.begin();
					while (it != branch.end())
					{
						if (*it == child)
							break;
						++it;
					}

					if (it == branch.end())
					{
						std::list<const Symbol *> subBranch = branch;
						subBranch.push_back(child);
						branches.push_back(subBranch);
					}
				}

				branches.pop_front();
			}
		}

		return cycles;
	}

	Cycles Registry::computeScc(const ComputeCyclesParameters &parameters) const
	{
		// https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm

		SccContext context(parameters, _symbols);

		for (auto &pair : _symbols)
		{
			auto it = context.meta.find(pair.first);
			if (it == context.meta.end())
			{
				Symbol *symbol = pair.second;
				strongConnect(symbol, context);
			}
		}

		return context.cluters;
	}

	bool Registry::operator==(const Registry &other) const
	{
		if (_symbols.size() != other._symbols.size())
			return false;

		for (auto itSymbol = _symbols.begin(), itOtherSymbol = other._symbols.begin();
			itSymbol != _symbols.end();
			++itSymbol, ++itOtherSymbol)
		{
			const Symbol *symbol = itSymbol->second;
			const Symbol *otherSymbol = itOtherSymbol->second;

			if (symbol->type != otherSymbol->type)
				return false;
			if (symbol->defined != otherSymbol->defined)
				return false;

			if (symbol->identifier.name != otherSymbol->identifier.name)
				return false;
			if (symbol->identifier.type != otherSymbol->identifier.type)
				return false;
			
			if (symbol->templateParameters.size() != otherSymbol->templateParameters.size())
				return false;

			for (auto itTemplateParameterItem = symbol->templateParameters.begin(), itOtherTemplateParameterItem = otherSymbol->templateParameters.begin();
			itTemplateParameterItem != symbol->templateParameters.end();
				++itTemplateParameterItem, ++itOtherTemplateParameterItem)
			{
				if (*itTemplateParameterItem != *itOtherTemplateParameterItem)
					return false;
			}

			const Namespace *iterNs = symbol->ns;
			const Namespace *iterOtherNs = otherSymbol->ns;
			for (;;)
			{
				if (iterNs == &rootNameSpace && iterOtherNs == &other.rootNameSpace)
					break;

				if (iterNs->name != iterOtherNs->name)
					return false;

				iterNs = iterNs->parent;
				iterOtherNs = iterOtherNs->parent;
			}

			if (symbol->references.size() != otherSymbol->references.size())
				return false;

			for (auto itReferenceItem = symbol->references.begin(), itOtherReferenceItem = otherSymbol->references.begin();
				itReferenceItem != symbol->references.end();
				++itReferenceItem, ++itOtherReferenceItem)
			{
				if (itReferenceItem->first != itOtherReferenceItem->first)
					return false;

				if (itReferenceItem->second.size() != itOtherReferenceItem->second.size())
					return false;

				for (auto itReference = itReferenceItem->second.begin(), itOtherReference = itOtherReferenceItem->second.begin();
				itReference != itReferenceItem->second.end();
					++itReference, ++itOtherReference)
				{
					const Reference &reference = *itReference;
					const Reference &otherReference = *itOtherReference;

					if (reference.type != otherReference.type)
						return false;
					if (!reference.location.filename.empty() &&
						!otherReference.location.filename.empty() &&
						reference.location.filename != otherReference.location.filename)
						return false;
					if (reference.location.line != otherReference.location.line)
						return false;
					if (reference.location.column != otherReference.location.column)
						return false;
				}
			}
		}

		return true;
	}
}
