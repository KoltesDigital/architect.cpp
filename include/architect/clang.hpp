#pragma once
#ifdef ARCHITECT_CLANG_SUPPORT

#include <functional>
#include <string>

typedef struct CXTranslationUnitImpl *CXTranslationUnit;

namespace architect
{
	class Registry;

	namespace clang
	{
		typedef std::function<bool(const std::string &)> Filter;

		struct Parameters
		{
			Filter filter; // returns whether to visit symbols in the file

			Parameters();
		};

		class DirectoryFilter
		{
		public:
			DirectoryFilter(); // working directory
			DirectoryFilter(const std::string &path);

			bool operator()(const std::string &filename) const;

		private:
			std::string _path;
		};

		void parse(Registry &registry, const CXTranslationUnit translationUnit, Parameters &parameters = Parameters());

		bool parse(Registry &registry, int argc, const char *const *argv, Parameters &parameters = Parameters());
	}
}

#endif
