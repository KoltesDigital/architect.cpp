configurations {
	"Release",
	"Debug",
}

platforms {
	"x32",
	"x64",
}

formats = {
	clang = true,
	console = true,
	dot = true,
	json = true,
}

flags {
	"FatalWarnings",
	"MultiProcessorCompile",
	"NoPCH",
	"Unicode",
}

defines {
	-- "ARCHITECT_CLANG_PRINT_CURSORS", -- for debugging cursor traversal
}

if formats.clang then defines { "ARCHITECT_CLANG_SUPPORT" } end
if formats.console then defines { "ARCHITECT_CONSOLE_SUPPORT" } end
if formats.dot then defines { "ARCHITECT_DOT_SUPPORT" } end
if formats.json then defines { "ARCHITECT_JSON_SUPPORT" } end

filter "configurations:Debug"
	defines { "DEBUG" }
	flags { "Symbols" }
	targetsuffix "-d"

filter "configurations:Release"
	defines { "NDEBUG" }
	optimize "On"

filter { "platforms:x32" }
	architecture "x32"
	libdirs { "dep/lib" }
	targetdir "bin"

filter { "platforms:x64" }
	architecture "x64"
	libdirs { "dep/lib64" }
	targetdir "bin64"

workspace "architect"
	language "C++"
	location "build"

project "library"
	files {
		"code/library/**",
		"include/**",
	}
	includedirs {
		"include",
		"dep/include",
	}
	location "build"
	kind "StaticLib"
	rtti "Off"
	targetname "architect"

	filter { "platforms:x32" }
		targetdir "lib"

	filter { "platforms:x64" }
		targetdir "lib64"


project "cli"
	files {
		"code/cli/**",
	}
	includedirs {
		"include",
		"dep/include",
	}
	links {
		"library",
	}
	if formats.clang then links { "libclang" } end
	location "build"
	kind "ConsoleApp"
	rtti "Off"
	targetname "architect"

project "tests-runner"
	files {
		"code/tests-runner/**",
	}
	includedirs {
		"include",
		"dep/include",
	}
	links {
		"library",
	}
	if formats.clang then links { "libclang" } end
	location "build"
	kind "ConsoleApp"
	rtti "Off"
	targetname "tests"

project "tests"
	files {
		"tests/**",
	}
	kind "None"
