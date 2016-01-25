#ifdef ARCHITECT_JSON_SUPPORT
#include <architect/json.hpp>

#include <fstream>
#include <list>
#include <string>
#include <architect/Registry.hpp>
#include <architect/Symbol.hpp>

using _json = nlohmann::json;

namespace architect
{
	namespace
	{
		bool getProperty(const _json &j, const std::string &name, _json &value)
		{
			auto it = j.find(name);
			if (it == j.end())
				return false;
			value = *it;
			return true;
		}

		bool getProperty(const _json &j, const std::string &name, _json::boolean_t &value)
		{
			_json jField;
			if (!getProperty(j, name, jField) || !jField.is_boolean())
				return false;
			value = jField.get<_json::boolean_t>();
			return true;
		}

		bool getProperty(const _json &j, const std::string &name, _json::number_integer_t &value)
		{
			_json jField;
			if (!getProperty(j, name, jField) || !jField.is_number_integer())
				return false;
			value = jField.get<_json::number_integer_t>();
			return true;
		}

		bool getProperty(const _json &j, const std::string &name, _json::string_t &value)
		{
			_json jField;
			if (!getProperty(j, name, jField) || !jField.is_string())
				return false;
			value = jField.get<_json::string_t>();
			return true;
		}

		_json dumpReferenceType(ReferenceType type)
		{
			switch (type)
			{
			case ReferenceType::TEMPLATE:
				return "template";
			case ReferenceType::INHERITANCE:
				return "inheritance";
			case ReferenceType::COMPOSITION:
				return "composition";
			case ReferenceType::ASSOCIATION:
				return "association";
			default:
				return "???";
			}
		}

		bool parseReferenceType(const _json &j, ReferenceType &type)
		{
			auto &str = j.get<_json::string_t>();
			if (str == "template")
			{
				type = ReferenceType::TEMPLATE;
				return true;
			}
			if (str == "inheritance")
			{
				type = ReferenceType::INHERITANCE;
				return true;
			}
			if (str == "composition")
			{
				type = ReferenceType::COMPOSITION;
				return true;
			}
			if (str == "association")
			{
				type = ReferenceType::ASSOCIATION;
				return true;
			}
			return false;
		}

		_json dumpSymbolType(SymbolType type)
		{
			switch (type)
			{
			case SymbolType::GLOBAL:
				return "global";
			case SymbolType::GLOBAL_TEMPLATE:
				return "globalTemplate";
			case SymbolType::RECORD:
				return "record";
			case SymbolType::RECORD_TEMPLATE:
				return "recordTemplate";
			case SymbolType::TYPEDEF:
				return "typedef";
			default:
				return "???";
			}
		}

		bool parseSymbolType(const _json &j, SymbolType &type)
		{
			auto &str = j.get<_json::string_t>();
			if (str == "global")
			{
				type = SymbolType::GLOBAL;
				return true;
			}
			if (str == "globalTemplate")
			{
				type = SymbolType::GLOBAL_TEMPLATE;
				return true;
			}
			if (str == "record")
			{
				type = SymbolType::RECORD;
				return true;
			}
			if (str == "recordTemplate")
			{
				type = SymbolType::RECORD_TEMPLATE;
				return true;
			}
			if (str == "typedef")
			{
				type = SymbolType::TYPEDEF;
				return true;
			}
			return false;
		}

		_json getBasicSymbol(const Symbol *symbol)
		{
			_json jIdentifier = _json::object();
			jIdentifier["name"] = symbol->identifier.name;
			jIdentifier["type"] = symbol->identifier.type;

			std::list<std::string> namespaces;
			Namespace *iterNs = symbol->ns;
			while (iterNs->parent)
			{
				namespaces.push_front(iterNs->name);
				iterNs = iterNs->parent;
			}

			_json jSymbol = _json::object();
			jSymbol["type"] = dumpSymbolType(symbol->type);
			jSymbol["defined"] = symbol->defined;
			jSymbol["identifier"] = jIdentifier;

			if (!symbol->templateParameters.empty())
				jSymbol["templateParameters"] = symbol->templateParameters;

			if (!namespaces.empty())
				jSymbol["namespaces"] = namespaces;

			return jSymbol;
		}
	}

	namespace json
	{
		FormattingParameters::FormattingParameters()
			: pretty(false)
		{}

		bool parse(Registry &registry, const _json &j)
		{
			if (!j.is_array())
				return false;

			auto &symbols = registry.getSymbols();

			for (auto &jSymbol : j)
			{
				if (!jSymbol.is_object())
					return false;

				_json::string_t jType;
				if (!getProperty(jSymbol, "type", jType))
					return false;
				SymbolType type;
				if (!parseSymbolType(jType, type))
					return false;

				_json::boolean_t jDefined;
				if (!getProperty(jSymbol, "defined", jDefined))
					return false;

				Symbol *symbol = registry.createSymbol(type, jDefined);

				_json jIdentifier;
				if (!getProperty(jSymbol, "identifier", jIdentifier) || !jIdentifier.is_object())
					return false;
				if (!getProperty(jIdentifier, "name", symbol->identifier.name))
					return false;
				if (!getProperty(jIdentifier, "type", symbol->identifier.type))
					return false;

				_json jTemplateParameters;
				if (getProperty(jSymbol, "templateParameters", jTemplateParameters))
				{
					if (!jTemplateParameters.is_array())
						return false;

					for (auto &jTemplateParameter : jTemplateParameters)
					{
						if (!jTemplateParameter.is_string())
							return false;
						symbol->templateParameters.push_back(jTemplateParameter.get<_json::string_t>());
					}
				}

				_json jNamespaces;
				Namespace *iterNs = &registry.rootNameSpace;
				if (getProperty(jSymbol, "namespaces", jNamespaces))
				{
					if (!jNamespaces.is_array())
						return false;

					for (auto &jNamespace : jNamespaces)
					{
						if (!jNamespace.is_string())
							return false;
						std::string name = jNamespace.get<_json::string_t>();

						auto it = iterNs->children.find(name);
						if (it == iterNs->children.end())
						{
							Namespace *ns = registry.createNamespace();
							ns->name = name;

							ns->parent = iterNs;
							it = iterNs->children.insert(std::pair<std::string, Namespace *>(name, ns)).first;
						}

						iterNs = it->second;
					}
					iterNs->symbols.insert(std::pair<SymbolIdentifier, Symbol *>(symbol->identifier, symbol));
				}
				symbol->ns = iterNs;

				_json jReferences;
				if (!getProperty(jSymbol, "references", jReferences) || !jReferences.is_array())
					return false;
				for (auto &jReferenceItem : jReferences)
				{
					_json::number_integer_t id;
					if (!getProperty(jReferenceItem, "id", id))
						return false;

					auto &referenceSet = symbol->references[(SymbolId)id];

					_json jReferenceSet;
					if (!getProperty(jReferenceItem, "references", jReferenceSet) || !jReferenceSet.is_array())
						return false;
					for (auto &jReference : jReferenceSet)
					{
						Reference reference;

						_json::string_t jType;
						if (!getProperty(jReference, "type", jType))
							return false;
						if (!parseReferenceType(jType, reference.type))
							return false;

						if (!getProperty(jReference, "filename", reference.location.filename))
							return false;

						_json::number_integer_t intLine;
						if (!getProperty(jReference, "line", intLine))
							return false;
						reference.location.line = (uint32_t)intLine;

						_json::number_integer_t intColumn;
						if (!getProperty(jReference, "column", intColumn))
							return false;
						reference.location.column = (uint32_t)intColumn;

						referenceSet.insert(reference);
					}
				}
			}

			return true;
		}

		bool parse(Registry &registry, std::istream &stream)
		{
			std::ostringstream buffer;
			buffer << stream.rdbuf();

			auto j = _json::parse(buffer.str());
			return parse(registry, j);
		}

		void dumpCycles(const Cycles &cycles, nlohmann::json &j, const FormattingParameters &parameters)
		{
			_json::array_t jCycles(cycles.size());
			size_t cycleIndex = 0;
			for (auto &cycle : cycles)
			{
				_json::array_t jCycle(cycle.size());
				size_t symbolIndex = 0;
				for (auto symbol : cycle)
				{
					_json jSymbol = getBasicSymbol(symbol);
					jCycle[symbolIndex] = jSymbol;
					++symbolIndex;
				}

				jCycles[cycleIndex] = jCycle;
				++cycleIndex;
			}

			j = jCycles;
		}

		void dumpCycles(const Cycles &cycles, std::ostream &stream, const FormattingParameters &parameters)
		{
			_json j;
			dumpCycles(cycles, j, parameters);
			stream << j.dump(parameters.pretty ? 2 : -1) << "\n";
		}

		void dumpSymbols(const Symbols &symbols, nlohmann::json &j, const FormattingParameters &parameters)
		{
			_json::array_t jSymbols(symbols.size());

			size_t index = 0;
			for (auto &pair : symbols)
			{
				Symbol *symbol = pair.second;

				std::list<_json> references;
				for (auto &pair : symbol->references)
				{
					_json jReferenceSet = _json::array();
					for (auto &reference : pair.second)
					{
						_json jReference;
						jReference["type"] = dumpReferenceType(reference.type);
						jReference["filename"] = reference.location.filename;
						jReference["line"] = reference.location.line;
						jReference["column"] = reference.location.column;

						jReferenceSet.push_back(jReference);
					}

					_json jReferences = _json::object();
					jReferences["id"] = pair.first;
					jReferences["references"] = jReferenceSet;

					references.push_back(jReferences);
				}

				_json jSymbol = getBasicSymbol(symbol);
				jSymbol["references"] = references;

				jSymbols[index] = jSymbol;

				++index;
			}

			j = jSymbols;
		}

		void dumpSymbols(const Symbols &symbols, std::ostream &stream, const FormattingParameters &parameters)
		{
			_json j;
			dumpSymbols(symbols, j, parameters);
			stream << j.dump(parameters.pretty ? 2 : -1) << "\n";
		}
	}
}

#endif
