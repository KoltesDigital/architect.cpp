#include <architect/Symbol.hpp>

namespace architect
{
	namespace
	{
		std::string doubleColon("::");
	}

	bool SymbolIdentifier::operator<(const SymbolIdentifier &other) const
	{
		auto compare = name.compare(other.name);
		if (compare)
			return compare < 0;

		compare = type.compare(other.type);
		return compare < 0;
	}

	std::string Symbol::getFullName() const
	{
		std::string name;
		switch (type)
		{
		case SymbolType::GLOBAL:
		{
			name = identifier.type + " " + identifier.name;
			break;
		}

		default:
			name = identifier.name;
			break;
		}

		Namespace *iterNs = ns;
		while (iterNs->parent)
		{
			name = iterNs->name + doubleColon + name;
			iterNs = iterNs->parent;
		}

		return name;
	}
}
