#ifdef ARCHITECT_CLANG_SUPPORT
#include <architect/clang.hpp>

#include <clang-c/Index.h>
#include <architect/Registry.hpp>
#include <architect/Symbol.hpp>

namespace architect
{
	namespace
	{
		CXChildVisitResult debugVisitor(CXCursor cursor, CXCursor parent, CXClientData clientData)
		{
			std::string &prefix = *static_cast<std::string *>(clientData);
			std::string subPrefix = prefix + "    ";

			CXCursorKind kind = clang_getCursorKind(cursor);
			std::string kindName(clang_getCString(clang_getCursorKindSpelling(kind))); // debug

			CXType type = clang_getCursorType(cursor);
			std::string typeName(clang_getCString(clang_getTypeSpelling(type)));
			std::string typeKindName(clang_getCString(clang_getTypeKindSpelling(type.kind)));

			std::string name(clang_getCString(clang_getCursorSpelling(cursor)));
			std::string displayName(clang_getCString(clang_getCursorDisplayName(cursor)));

			std::string usr(clang_getCString(clang_getCursorUSR(cursor)));

			int cursorNumTemplateArguments = clang_Cursor_getNumTemplateArguments(cursor);
			int typeNumTemplateArguments = clang_Type_getNumTemplateArguments(type);

			std::cout
				<< prefix << kindName << "\n"
				<< prefix << "    type spelling: " << typeName << "\n"
				<< prefix << "    type kind spelling: " << typeKindName << "\n"
				<< prefix << "    cursor spelling: " << name << "\n"
				<< prefix << "    cursor display name: " << displayName << "\n"
				<< prefix << "    cursor usr: " << usr << "\n"
				// << prefix << "  \ cursorNumTemplateArguments << ", " << typeNumTemplateArguments << ">"
				<< std::endl;

			//if (kind == CXCursor_TranslationUnit)
			//		return CXChildVisit_Break;

			if (typeNumTemplateArguments > 0)
			{
				std::cout << prefix << "  Template types:" << std::endl;
				for (int i = 0; i < typeNumTemplateArguments; ++i)
				{
					CXType argType = clang_Type_getTemplateArgumentAsType(type, i);
					std::string argTypeName(clang_getCString(clang_getTypeSpelling(argType)));
					std::string argTypeKindName(clang_getCString(clang_getTypeKindSpelling(argType.kind)));
					std::cout << subPrefix << argTypeName << " (" << argTypeKindName << ")" << std::endl;
				}
			}

			if (clang_isReference(kind))
			{
				std::cout << prefix << "  Reference";
				CXCursor refCursor = clang_getCursorReferenced(cursor);
				if (!clang_Cursor_isNull(refCursor))
				{
					std::string refName(clang_getCString(clang_getCursorSpelling(refCursor)));
					std::cout << ": " << refName << std::endl;

					//clang_visitChildren(refCursor, debugVisitor, clientData);
					//debugVisitor(refCursor, cursor, &subPrefix);
				}
				std::cout << std::endl;
			}

			CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
			if (!clang_Cursor_isNull(parentCursor))
			{
				std::string parentName(clang_getCString(clang_getCursorSpelling(parentCursor)));
				std::cout << prefix << "  Parent: " << parentName << std::endl;
			}

			clang_visitChildren(cursor, debugVisitor, &subPrefix);

			return CXChildVisit_Continue;
		}

		class VisitorContext
		{
		public:
			VisitorContext(Registry *registry, Namespace *nameSpace);

			VisitorContext setReferenceType(ReferenceType referenceType) const;

			VisitorContext declareNameSpace(const CXCursor &cursor);

			VisitorContext declareGlobal(const CXCursor &cursor, bool isTemplate, bool &wasDefined);

			VisitorContext declareRecord(const CXCursor &cursor, bool isTemplate, bool &wasDefined);

			VisitorContext declareTemplateParameter(const CXCursor &cursor);

			VisitorContext declareTypedef(const CXCursor &cursor);

			void declareReference(const CXCursor &cursor, const CXCursor &referenceCursor);

		private:
			Symbol *getSymbol(const CXCursor &cursor, SymbolIdentifier &identifier) const;

			Symbol *declareSymbol(SymbolType symbolType, const CXCursor &cursor, const CXCursor &referenceCursor, bool &wasDefined);

			static CXChildVisitResult nameSpaceVisitor(CXCursor cursor, CXCursor parent, CXClientData clientData);

			Registry *_registry;
			Namespace *_currentNameSpace;
			Symbol *_currentSymbol;
			ReferenceType _referenceType;
			bool _inRecord;
			bool _inTemplateParameter;
		};

		VisitorContext::VisitorContext(Registry *registry, Namespace *ns)
			: _registry(registry)
			, _currentNameSpace(ns)
			, _currentSymbol(nullptr)
			, _referenceType(ReferenceType::ASSOCIATION)
			, _inRecord(false)
			, _inTemplateParameter(false)
		{}

		VisitorContext VisitorContext::setReferenceType(ReferenceType referenceType) const
		{
			VisitorContext subContext(*this);
			subContext._referenceType = referenceType;
			return subContext;
		}

		VisitorContext VisitorContext::declareNameSpace(const CXCursor &cursor)
		{
			auto subNameSpace = _registry->createNamespace();
			subNameSpace->parent = _currentNameSpace;
			subNameSpace->name = clang_getCString(clang_getCursorSpelling(cursor));

			_currentNameSpace->children.insert(std::pair<std::string, Namespace *>(subNameSpace->name, subNameSpace));

			VisitorContext subContext(*this);
			subContext._currentNameSpace = subNameSpace;
			return subContext;
		}

		VisitorContext VisitorContext::declareGlobal(const CXCursor &cursor, bool isTemplate, bool &wasDefined)
		{
			VisitorContext subContext(*this);
			subContext._referenceType = ReferenceType::ASSOCIATION;

			if (!_inRecord)
			{
				Symbol *symbol = declareSymbol(isTemplate ? SymbolType::GLOBAL_TEMPLATE : SymbolType::GLOBAL, cursor, cursor, wasDefined);
				subContext._currentSymbol = symbol;
			}

			return subContext;
		}

		VisitorContext VisitorContext::declareRecord(const CXCursor &cursor, bool isTemplate, bool &wasDefined)
		{
			Symbol *symbol = declareSymbol(isTemplate ? SymbolType::RECORD_TEMPLATE : SymbolType::RECORD, cursor, cursor, wasDefined);

			VisitorContext subContext(*this);
			subContext._currentSymbol = symbol;
			subContext._referenceType = ReferenceType::COMPOSITION;
			subContext._inRecord = true;
			return subContext;
		}
		
		VisitorContext VisitorContext::declareTemplateParameter(const CXCursor &cursor)
		{
			if (_inTemplateParameter)
				return *this;

			auto name = clang_getCString(clang_getCursorSpelling(cursor));
			_currentSymbol->templateParameters.push_back(name);

			VisitorContext subContext(*this);
			subContext._referenceType = ReferenceType::ASSOCIATION;
			subContext._inTemplateParameter = true;
			return subContext;
		}

		VisitorContext VisitorContext::declareTypedef(const CXCursor &cursor)
		{
			bool wasDefined;
			Symbol *symbol = declareSymbol(SymbolType::TYPEDEF, cursor, cursor, wasDefined);

			VisitorContext subContext(*this);
			subContext._currentSymbol = symbol;
			return subContext;
		}

		void VisitorContext::declareReference(const CXCursor &cursor, const CXCursor &referenceCursor)
		{
			SymbolIdentifier identifier;
			Symbol *symbol = getSymbol(cursor, identifier);

			if (symbol && _currentSymbol && symbol != _currentSymbol)
			{
				Reference reference;
				reference.location.getFromCursor(referenceCursor);
				reference.type = _referenceType;

				auto &referenceSet = _currentSymbol->references[symbol->id];
				referenceSet.insert(reference);
			}
		}

		Symbol *VisitorContext::getSymbol(const CXCursor &cursor, SymbolIdentifier &identifier) const
		{
			CXType type = clang_getCursorType(cursor);

			identifier.name = clang_getCString(clang_getCursorSpelling(cursor));
			identifier.type = clang_getCString(clang_getTypeSpelling(type));

			switch (type.kind)
			{
			case CXType_Enum:
			case CXType_FunctionProto:
			case CXType_Invalid:
			case CXType_Record:
			case CXType_Typedef:
			{
				std::list<std::string> _namespaces;
				clang_visitChildren(cursor, nameSpaceVisitor, &_namespaces);

				Namespace *ns = _currentNameSpace;
				do
				{
					Namespace *finalNameSpace = ns;
					auto itName = _namespaces.begin();
					while (itName != _namespaces.end())
					{
						auto itChild = finalNameSpace->children.find(*itName);
						if (itChild == finalNameSpace->children.end())
							break;
						finalNameSpace = itChild->second;
						++itName;
					}

					if (itName == _namespaces.end())
					{
						auto it = finalNameSpace->symbols.find(identifier);
						if (it != finalNameSpace->symbols.end())
						{
							return it->second;
						}
					}

					ns = ns->parent;
				} while (ns);

				break;
			}
			}

			return nullptr;
		}

		Symbol *VisitorContext::declareSymbol(SymbolType symbolType, const CXCursor &cursor, const CXCursor &referenceCursor, bool &wasDefined)
		{
			SymbolIdentifier identifier;
			Symbol *symbol = getSymbol(cursor, identifier);
			if (!symbol)
			{
				symbol = _registry->createSymbol(symbolType, false);
				symbol->identifier = identifier;
				symbol->ns = _currentNameSpace;

				_currentNameSpace->symbols.insert(std::pair<SymbolIdentifier, Symbol *>(identifier, symbol));
			}

			wasDefined = symbol->defined;
			if (clang_isCursorDefinition(cursor))
				symbol->defined = true;

			if (_currentSymbol && symbol != _currentSymbol)
			{
				Reference reference;
				reference.location.getFromCursor(referenceCursor);
				reference.type = _referenceType;

				auto &referenceSet = _currentSymbol->references[symbol->id];
				referenceSet.insert(reference);
			}

			return symbol;
		}

		CXChildVisitResult VisitorContext::nameSpaceVisitor(CXCursor cursor, CXCursor parent, CXClientData clientData)
		{
			CXCursorKind kind = clang_getCursorKind(cursor);
			if (kind == CXCursor_NamespaceRef)
			{
				std::string name = clang_getCString(clang_getCursorSpelling(cursor));
				std::list<std::string> &_namespaces = *static_cast<std::list<std::string> *>(clientData);
				_namespaces.push_back(name);
			}
			return CXChildVisit_Continue;
		}

		CXChildVisitResult functionVisitor(CXCursor cursor, CXCursor parent, CXClientData clientData);
		CXChildVisitResult globalVisitor(CXCursor cursor, CXCursor parent, CXClientData clientData);

		bool handleReference(CXCursor &cursor, VisitorContext &context)
		{
			auto name = clang_getCString(clang_getCursorSpelling(cursor));
			CXCursorKind kind = clang_getCursorKind(cursor);
			switch (kind)
			{
			case CXCursor_TemplateRef:
			{
				if (clang_isReference(kind))
				{
					CXCursor referencedCursor = clang_getCursorReferenced(cursor);
					if (!clang_Cursor_isNull(referencedCursor))
					{
						globalVisitor(referencedCursor, cursor, &context);
					}
				}
				break;
			}

			case CXCursor_TypeRef:
			{
				if (clang_isReference(kind))
				{
					CXCursor referencedCursor = clang_getCursorReferenced(cursor);
					if (!clang_Cursor_isNull(referencedCursor))
					{
						context.declareReference(referencedCursor, cursor);
					}
				}
				break;
			}

			case CXCursor_TemplateTypeParameter:
			case CXCursor_NonTypeTemplateParameter:
			case CXCursor_TemplateTemplateParameter:
			{
				VisitorContext subContext = context.declareTemplateParameter(cursor);
				clang_visitChildren(cursor, functionVisitor, &subContext);
				break;
			}

			default:
				return false;
			}
			return true;
		}

		CXChildVisitResult functionVisitor(CXCursor cursor, CXCursor parent, CXClientData clientData)
		{
			VisitorContext &context = *static_cast<VisitorContext *>(clientData);

			if (handleReference(cursor, context))
				return CXChildVisit_Continue;

			auto name = clang_getCString(clang_getCursorSpelling(cursor));
			CXCursorKind kind = clang_getCursorKind(cursor);
			switch (kind)
			{
			case CXCursor_DeclRefExpr:
			{
				context.declareReference(cursor, cursor);
				break;
			}
			}

			return CXChildVisit_Recurse;
		}

		CXChildVisitResult globalVisitor(CXCursor cursor, CXCursor parent, CXClientData clientData)
		{
			VisitorContext &context = *static_cast<VisitorContext *>(clientData);

			if (handleReference(cursor, context))
				return CXChildVisit_Continue;

			auto name = clang_getCString(clang_getCursorSpelling(cursor));
			CXCursorKind kind = clang_getCursorKind(cursor);
			switch (kind)
			{
			case CXCursor_Namespace:
			{
				VisitorContext subContext = context.declareNameSpace(cursor);
				clang_visitChildren(cursor, globalVisitor, &subContext);
				break;
			}

			case CXCursor_ClassDecl:
			case CXCursor_ClassTemplate:
			case CXCursor_EnumDecl:
			case CXCursor_StructDecl:
			case CXCursor_UnionDecl:
			{
				bool wasDefined;
				VisitorContext subContext = context.declareRecord(cursor, kind == CXCursor_ClassTemplate, wasDefined);
				if (!wasDefined)
					clang_visitChildren(cursor, globalVisitor, &subContext);
				break;
			}

			case CXCursor_TypedefDecl:
			{
				VisitorContext subContext = context.declareTypedef(cursor);
				clang_visitChildren(cursor, globalVisitor, &subContext);
				break;
			}

			case CXCursor_FunctionDecl:
			case CXCursor_FunctionTemplate:
			case CXCursor_VarDecl:
			{
				bool wasDefined;
				VisitorContext subContext = context.declareGlobal(cursor, kind == CXCursor_FunctionTemplate, wasDefined);
				if (!wasDefined)
					clang_visitChildren(cursor, functionVisitor, &subContext);
				break;
			}

			case CXCursor_CXXMethod:
			{
				VisitorContext subContext = context.setReferenceType(ReferenceType::ASSOCIATION);
				clang_visitChildren(cursor, functionVisitor, &subContext);
				break;
			}

			case CXCursor_FieldDecl:
			{
				return CXChildVisit_Recurse;
			}

			case CXCursor_CXXBaseSpecifier:
			{
				VisitorContext subContext = context.setReferenceType(ReferenceType::INHERITANCE);
				clang_visitChildren(cursor, functionVisitor, &subContext);
				break;
			}
			}

			return CXChildVisit_Continue;
		}
	}

	namespace clang
	{
		void parse(Registry &registry, const CXTranslationUnit translationUnit)
		{
			CXCursor rootCursor = clang_getTranslationUnitCursor(translationUnit);

			//std::string prefix;
			//clang_visitChildren(rootCursor, debugVisitor, &prefix);

			VisitorContext context(&registry, &registry.rootNameSpace);
			clang_visitChildren(rootCursor, globalVisitor, &context);
		}

		bool parse(Registry &registry, int argc, const char *const *argv)
		{
			CXIndex index = clang_createIndex(0, 0);
			if (!index)
			{
				return false;
			}

			CXTranslationUnit translationUnit = clang_parseTranslationUnit(index, 0,
				argv, argc, 0, 0, CXTranslationUnit_None);
			if (!translationUnit)
			{
				return false;
			}

			parse(registry, translationUnit);

			clang_disposeTranslationUnit(translationUnit);
			clang_disposeIndex(index);

			return true;
		}
	}
}

#endif
