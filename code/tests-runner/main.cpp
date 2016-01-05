#include <fstream>
#include <list>
#include <json.hpp>
#include <tinydir.h>
#include <architect.hpp>

using json = nlohmann::json;

const char *testArgv[2];

bool test(const char *testName)
{
#ifdef ARCHITECT_CLANG_SUPPORT
	testArgv[1] = testName;

	architect::Registry actualRegistry;
	if (!architect::clang::parse(actualRegistry, 2, testArgv))
		return false;

	std::string jsonFilename(testName);
	jsonFilename += ".json";
	architect::Registry expectedRegistry;

	std::ifstream ifile(jsonFilename);
	if (!ifile.is_open())
	{
		std::ofstream ofile(jsonFilename);
		if (!ofile.is_open())
			return false;

		architect::json::FormattingParameters parameters;
		parameters.pretty = true;
		architect::json::dumpSymbols(actualRegistry.getSymbols(), ofile, parameters);
		return true;
	}

	if (!architect::json::parse(expectedRegistry, ifile))
		return false;
	
	return actualRegistry == expectedRegistry;
#else
	return false;
#endif
}

int main(int argc, char **argv)
{
	testArgv[0] = argv[0];

	int errors = 0;

	tinydir_dir dir;
	tinydir_open(&dir, ".");

	while (dir.has_next)
	{
		tinydir_file file;
		tinydir_readfile(&dir, &file);

		if (!file.is_dir)
		{
			if (!strcmp(file.extension, "cpp"))
			{
				const char *testName = file.path;
				bool succeeded = test(testName);

				std::cout << file.name << ": ";

				if (succeeded)
					std::cout << "SUCCEEDED";
				else
				{
					++errors;
					std::cout << "FAILED";
				}

				std::cout << std::endl;
			}
		}

		tinydir_next(&dir);
	}

	tinydir_close(&dir);

	return errors;
}
