#pragma once
#ifdef ARCHITECT_CLANG_SUPPORT

typedef struct CXTranslationUnitImpl *CXTranslationUnit;

namespace architect
{
	class Registry;

	namespace clang
	{
		struct Parameters
		{
			bool workingDirectory;

			Parameters()
				: workingDirectory(false)
			{}
		};

		void parse(Registry &registry, const CXTranslationUnit translationUnit, Parameters &parameters = Parameters());

		bool parse(Registry &registry, int argc, const char *const *argv, Parameters &parameters = Parameters());
	}
}

#endif
