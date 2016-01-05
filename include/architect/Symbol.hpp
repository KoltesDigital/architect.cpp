#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>
#include <architect/Reference.hpp>

namespace architect
{
	struct SymbolIdentifier
	{
		bool operator<(const SymbolIdentifier &other) const;

		std::string name;
		std::string type;
	};

	typedef unsigned int SymbolId;

	struct Symbol;

	struct Namespace
	{
		Namespace *parent;
		std::map<std::string, Namespace *> children;

		std::map<SymbolIdentifier, Symbol *> symbols;

		std::string name;
	};

	typedef std::map<SymbolId, std::set<Reference>> References;

	enum class SymbolType
	{
		GLOBAL,
		GLOBAL_TEMPLATE,
		RECORD,
		RECORD_TEMPLATE,
		TYPEDEF,
	};

	struct Symbol
	{
		Namespace *ns;
		References references;

		SymbolId id;
		SymbolType type;
		bool defined;
		
		SymbolIdentifier identifier;
		std::vector<std::string> templateParameters;

		std::string getFullName() const;
	};
}
