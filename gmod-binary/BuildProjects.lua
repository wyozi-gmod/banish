solution "gmsv_banish"

	language "C++"
	location ( os.get() .."-".. _ACTION )
	flags { "Symbols", "NoEditAndContinue", "NoPCH", "StaticRuntime", "EnableSSE" }
	targetdir ( "lib/" .. os.get() .. "/" )
	includedirs { "include/", "../banish-rs/include/" }

	if os.is("windows") then
		-- "msvcrt" already included??
		links { "kernel32", "ws2_32", "userenv", "shell32", "advapi32" }
		links { "../banish-rs/target/i686-pc-windows-msvc/debug/banish" }
	end

	
	configurations
	{ 
		"Release"
	}
	
	configuration "Release"
		defines { "NDEBUG" }
		flags{ "Optimize", "FloatFast" }
	
	project "gmsv_banish"
		defines { "GMMODULE" }
		files { "src/**.*", "include/**.*", "../include/**.*" }
		kind "SharedLib"
		