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
		std::string name = identifier.name;
		if (name.empty())
			name = "?";

		Namespace *iterNs = ns;
		while (iterNs->parent)
		{
			name = iterNs->name + doubleColon + name;
			iterNs = iterNs->parent;
		}

		if (!templateParameters.empty())
		{
			name += "<";
			bool notFirst = false;
			for (auto &param : templateParameters)
			{
				if (notFirst)
					name += ", ";
				else
					notFirst = true;
				name += param;
			}
			name += ">";
		}

		if (type == SymbolType::GLOBAL ||
			type == SymbolType::GLOBAL_TEMPLATE)
			name = identifier.type + " " + name;

		return name;
	}
}
