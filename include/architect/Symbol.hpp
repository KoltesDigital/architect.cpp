#pragma once

#include <map>
#include <set>
#include <string>
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
		RECORD,
		TYPEDEF,
	};

	struct Symbol
	{
		bool operator==(const Symbol &other) const;

		Namespace *ns;
		References references;

		SymbolId id;
		SymbolType type;

		SymbolIdentifier identifier;

		std::string getFullName() const;
	};
}
