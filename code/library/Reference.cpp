#include <architect/Reference.hpp>

namespace architect
{
	bool Reference::operator<(const Reference &other) const
	{
		if (type != other.type)
			return type < other.type;

		return location < other.location;
	}
}
