#pragma once

#include <architect/Location.hpp>

namespace architect
{
	// most important first
	enum class ReferenceType
	{
		TEMPLATE,
		INHERITANCE,
		COMPOSITION,
		ASSOCIATION,
	};

	struct Reference
	{
		bool operator<(const Reference &other) const;

		Location location;
		ReferenceType type;
	};
}
