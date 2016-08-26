# architect.cpp

Tools for checking and visualizing C++ code architecture, both as library and command-line.

## Library

```cpp
#include <iostream>
#include <architect.hpp>

int main(int argc, const char **argv)
{
	architect::Registry registry;
	if (!architect::clang::parse(registry, argc, argv))
		return EXIT_FAILURE;

    auto &symbols = registry.getSymbols();
    architect::dot::dumpSymbols(symbols, std::cout);

    return EXIT_SUCCESS;
}
```

The central class is `architect::Registry` and its main method is `getSymbols`. A registry holds information on code's *symbols* (logical abstractions like classes, enums, typedefs, global functions...) and their references to other symbols. From there, everything is possible.

A registry is not meant to be changed directly. Parsing files, which fills the registry, or dumping it, is done through format namespaces (see *Input/output formats* below).

## Command-line

Simple interface on top of the library.

```console
architect -h  # Show general help
architect dependencies -h  # Show help for the command "dependencies"
architect dependencies tests\cycles.cpp  # Extract dependencies from files
```

Commands:

* `cycles`: shows all existing dependency cycles
* `dependencies`: shows symbols and their references to other symbols
* `scc`: shows the [strongly connected components](https://en.wikipedia.org/wiki/Strongly_connected_component) of the dependency graph

## Build

The build system is [Premake 5](https://premake.github.io/).

Edit `premake5.lua` if needed to accommodate your needs.

## Input/output formats

Several formats are supported: they are in the corresponding sub-namespaces of `architect`. With the command-line, specify the input and output formats using respectively `-input <format>` and `-output <format>`.

* `clang`: parses with [clang](http://clang.llvm.org/)
* `console`: displays with a basic formatting for development purpose
* `dot`: displays in [DOT](http://www.graphviz.org/)
* `json`: parses and displays in [JSON](http://json.org/)

Each of these formats can be opted out for building the library by editing `premake5.lua`. When using the library, you need to define the corresponding constants to access the namespace definitions.

## Provided dependencies

* [LLVM+Clang](http://llvm.org/) 3.7.1
* https://github.com/nlohmann/json c0132232768458b1a4b6eb6fb77345ee00b01797
* [cli.cpp](https://github.com/KoltesDigital/cli.cpp) a4d458887ce6e1ef2ff36ecd1e0525072186b29a

## License

Copyright (c) 2016 Jonathan Giroux

[MIT License](https://opensource.org/licenses/MIT)
