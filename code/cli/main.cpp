#include <fstream>
#include <iostream>
#include <architect.hpp>
#include <cli.hpp>

enum class Format
{
	DEFAULT,
	CLANG,
	CONSOLE,
	DOT,
	JSON,
	UNKNOWN,
};

Format getFormat(const char *option)
{
	if (!option)
		return Format::DEFAULT;
	if (!strcmp(option, "clang"))
		return Format::CLANG;
	if (!strcmp(option, "console"))
		return Format::CONSOLE;
	if (!strcmp(option, "dot"))
		return Format::DOT;
	if (!strcmp(option, "json"))
		return Format::JSON;
	return Format::UNKNOWN;
}

Format inputFormat;
bool workingDirectory;

bool loadRegistry(architect::Registry &registry, int argc, const char **argv)
{
	switch (inputFormat)
	{
	case Format::DEFAULT:

#ifdef ARCHITECT_CLANG_SUPPORT
	case Format::CLANG:
	{
		architect::clang::Parameters parameters;
		parameters.workingDirectory = workingDirectory;

		if (!architect::clang::parse(registry, argc, argv, parameters))
		{
			std::cerr << "Unable to parse" << std::endl;
			return false;
		}
		return true;
	}
#endif

#ifdef ARCHITECT_JSON_SUPPORT
	case Format::JSON:
		for (int i = 1; i < argc; ++i)
		{
			std::ifstream file(argv[i]);
			if (!file.is_open())
			{
				std::cerr << "Cannot open file " << argv[i] << std::endl;
				return false;
			}

			if (!architect::json::parse(registry, file))
			{
				std::cerr << "Unable to parse " << argv[i] << std::endl;
				return false;
			}
		}
		return true;
#endif

	default:
		std::cerr << "Unsupported input format for this command" << std::endl;
		return false;
	}
}

int main(int argc, const char **argv)
{
	architect::Registry registry;

	cli::Parser parser(argc, argv);

	auto help = parser.defaultHelpFlag();

	workingDirectory = parser.flag("working-directory")
		.alias("wd")
		.description("Restrict symbol definitions to working directory and subdirectories")
		.getValue();

	auto input = parser.option("input")
		.alias("i")
		.description("Set input format")
		.getValue();
	inputFormat = getFormat(input);
	if (inputFormat == Format::UNKNOWN)
		parser.reportError("Unknown input format: %s", input);

	auto output = parser.option("output")
		.alias("o")
		.description("Set output format")
		.getValue();
	auto outputFormat = getFormat(output);
	if (outputFormat == Format::UNKNOWN)
		parser.reportError("Unknown output format: %s", output);

	parser.command("cycles")
		.alias("c")
		.description("Show dependency cycles")
		.execute([&](cli::Parser &parser)
	{
		parser.help()
			<< R"(Show dependency cycles
Usage: cycles [options])";

		uint32_t minCardinalty = parser.option("min")
			.alias("m")
			.defaultValue("0")
			.description("Minimum cluster cardinality")
			.getValueAs<uint32_t>();

		bool pretty = parser.flag("pretty")
			.alias("p")
			.description("Pretty print with indentations and line returns")
			.getValue();

		parser.getRemainingArguments(argc, argv);
		if (!loadRegistry(registry, argc, argv))
			return EXIT_FAILURE;

		architect::ComputeCyclesParameters parameters;
		parameters.minCardinality = minCardinalty;
		auto cycles = registry.computeCycles(parameters);

		switch (outputFormat)
		{
		case Format::DEFAULT:

#ifdef ARCHITECT_CONSOLE_SUPPORT
		case Format::CONSOLE:
			architect::console::dumpCycles(cycles, std::cout);
			break;
#endif

#ifdef ARCHITECT_DOT_SUPPORT
		case Format::DOT:
		{
			architect::dot::FormattingParameters parameters;
			parameters.pretty = pretty;
			architect::dot::dumpCycles(cycles, std::cout, parameters);
			break;
		}
#endif

#ifdef ARCHITECT_JSON_SUPPORT
		case Format::JSON:
		{
			architect::json::FormattingParameters parameters;
			parameters.pretty = pretty;
			architect::json::dumpCycles(cycles, std::cout, parameters);
			break;
		}
#endif

		default:
			std::cerr << "Unsupported output format for this command" << std::endl;
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	});

	parser.command("dependencies")
		.alias("d")
		.description("Show dependencies")
		.execute([&](cli::Parser &parser)
		{
			parser.help()
				<< R"(Show dependencies
Usage: dependencies [options])";

			bool displayReferenceCount = parser.flag("reference-count")
				.alias("rc")
				.description("Display reference count on edges")
				.getValue();

			bool pretty = parser.flag("pretty")
				.alias("p")
				.description("Pretty print with indentations and line returns")
				.getValue();

			parser.getRemainingArguments(argc, argv);
			if (!loadRegistry(registry, argc, argv))
				return EXIT_FAILURE;

			auto &symbols = registry.getSymbols();

			switch (outputFormat)
			{
			case Format::DEFAULT:

#ifdef ARCHITECT_CONSOLE_SUPPORT
			case Format::CONSOLE:
				architect::console::dumpSymbols(symbols, std::cout);
				break;
#endif

#ifdef ARCHITECT_DOT_SUPPORT
			case Format::DOT:
			{
				architect::dot::FormattingParameters parameters;
				parameters.displayReferenceCount = displayReferenceCount;
				parameters.pretty = pretty;
				architect::dot::dumpSymbols(symbols, std::cout, parameters);
				break;
			}
#endif

#ifdef ARCHITECT_JSON_SUPPORT
			case Format::JSON:
			{
				architect::json::FormattingParameters parameters;
				parameters.pretty = pretty;
				architect::json::dumpSymbols(symbols, std::cout, parameters);
				break;
			}
#endif

			default:
				parser.reportError("Unsupported input format for this command: %s", input);
				break;
			}

			return EXIT_SUCCESS;
		});

	parser.command("scc")
		.description("Show strongly connect components")
		.execute([&](cli::Parser &parser)
	{
		parser.help()
			<< R"(Show strongly connect components
Usage: scc [options])";

		uint32_t minCardinalty = parser.option("min")
			.alias("m")
			.defaultValue("0")
			.description("Minimum cluster cardinality")
			.getValueAs<uint32_t>();

		bool pretty = parser.flag("pretty")
			.alias("p")
			.description("Pretty print with indentations and line returns")
			.getValue();

		parser.getRemainingArguments(argc, argv);
		if (!loadRegistry(registry, argc, argv))
			return EXIT_FAILURE;

		architect::ComputeCyclesParameters parameters;
		parameters.minCardinality = minCardinalty;
		auto cycles = registry.computeScc(parameters);

		switch (outputFormat)
		{
		case Format::DEFAULT:

#ifdef ARCHITECT_CONSOLE_SUPPORT
		case Format::CONSOLE:
			architect::console::dumpCycles(cycles, std::cout);
			break;
#endif

#ifdef ARCHITECT_DOT_SUPPORT
		case Format::DOT:
		{
			architect::dot::FormattingParameters parameters;
			parameters.pretty = pretty;
			architect::dot::dumpCycles(cycles, std::cout, parameters);
			break;
		}
#endif

#ifdef ARCHITECT_JSON_SUPPORT
		case Format::JSON:
		{
			architect::json::FormattingParameters parameters;
			parameters.pretty = pretty;
			architect::json::dumpCycles(cycles, std::cout, parameters);
			break;
		}
#endif

		default:
			std::cerr << "Unsupported output format for this command" << std::endl;
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	});

	if (parser.hasErrors())
		return EXIT_FAILURE;

	int result;
	if (!parser.executeCommand(result))
	{
		parser.help()
			<< R"(architect.cpp cli
Usage: <command> [options])";

		return EXIT_FAILURE;
	}

	return result;
}
