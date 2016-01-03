#pragma once
#ifdef ARCHITECT_CLANG_SUPPORT

typedef struct CXTranslationUnitImpl *CXTranslationUnit;

namespace architect
{
	class Registry;

	namespace clang
	{
		void parse(Registry &registry, const CXTranslationUnit translationUnit);
		bool parse(Registry &registry, int argc, const char *const *argv);
	}
}

#endif
