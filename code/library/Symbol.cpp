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
		case SymbolType::GLOBAL_TEMPLATE:
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

		return name;
	}
}
