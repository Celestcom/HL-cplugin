-- We are using PCH, so you must disable PCH on the protobuf files, and enable generation on stdafx.cpp


workspace "Plugin"
	configurations {"Debug", "Release"}
	platforms {"Win32", "Win64", "UnitTestWin32"}
	language "C++"
	
	


project "Plugin" 

	
	targetdir "bin/%{cfg.buildcfg}/%{cfg.platform}"
	targetname "NSLoader"
	
	-- dependencies
	protobuf_incl_dir = "D:/protobuf-3.0.0/cmake/build/solution/include"
	shared_comms_incl_dir = "C:/Users/NullSpace Team/Documents/NS_Unreal_SDK/src/Driver/SharedCommunication"
	boost_incl_dir = "D:/Libraries/boost/boost_1_61_0"
	protobuf_def_incl_dir = "C:/Users/NullSpace Team/Documents/NS_Unreal_SDK/src/Driver/protobuff_defs"
		disablewarnings {"4800"}


	includedirs {
		protobuf_incl_dir,
		shared_comms_incl_dir,
		boost_incl_dir,
		protobuf_def_incl_dir,
		"../src/",
		"../src/test",
		"../src/devices",
		"../src/events"
	}

	flags {
		"MultiProcessorCompile",
		"C++11"

	}
	--links {"System", "UnityEditor", "UnityEngine", "System.ServiceProcess"}

	files {
		"../src/**.cpp",
		"../src/**.h",
		path.join(protobuf_def_incl_dir, "DriverCommand.pb.cc"),
		path.join(protobuf_def_incl_dir, "EffectCommand.pb.cc"),
		path.join(shared_comms_incl_dir, "ScheduledEvent.cpp")
	}

	

	-- for protobuf


	boost_win32_dir = "D:/Libraries/boost/boost_1_61_0/stage/win32/lib"
	boost_win64_dir = "D:/Libraries/boost/boost_1_61_0/stage/x64/lib"

	protobuf_win32_dir = "D:/protobuf-3.0.0/cmake/build/solution"
	protobuf_win64_dir = "D:/protobuf-3.0.0/cmake/build/solution64"
	

	pchheader "stdafx.h"
	pchsource "../src/stdafx.cpp"


	defines {"NSLOADER_EXPORTS"}
	filter {"files:**.pb.cc"}
		flags {'NoPCH'}
	filter {"files:**ScheduledEvent.cpp"}
		flags {'NoPCH'}
	filter {"files:**test_main.cpp"}
	 	flags{'NoPCH'}


	filter {"platforms:Win32 or platforms:Win64"}
		kind "SharedLib"
	filter {"platforms:UnitTestWin32"}
		kind "ConsoleApp"

	-- input: libprotobuf
	filter {"platforms:Win32", "configurations:Debug"}
		libdirs {
			path.join(protobuf_win32_dir, "Debug")
		}
	filter {"platforms:Win32", "configurations:Release"}
		libdirs {
			path.join(protobuf_win32_dir, "Release")
		}
	filter {"platforms:Win64", "configurations:Debug"}
		libdirs {
			path.join(protobuf_win64_dir, "Debug")
		}
	filter {"platforms:Win64", "configurations:Release"}
		libdirs {
			path.join(protobuf_win64_dir, "Release")
		}

	-- unit testing
	filter {"platforms:UnitTestWin32", "configurations:Debug"}
		libdirs {
			path.join(protobuf_win32_dir, "Debug")
		}
	filter {"platforms:UnitTestWin32", "configurations:Release"}
		libdirs {
			path.join(protobuf_win32_dir, "Release")
		}


	filter "platforms:Win32 or platforms:UnitTestWin32" 
		system "Windows"
		architecture "x86"
		libdirs {
			boost_win32_dir
		}
		defines {"WIN32"}
	filter "platforms:Win64"
		system "Windows"
		architecture "x86_64"
		libdirs {
			boost_win64_dir
		}
	filter "configurations:Debug"
		defines {"DEBUG", "_DEBUG"}
		symbols "On"
		optimize "Off"
		links {"libprotobufd"}

	filter "configurations:Release"
		defines {"NDEBUG"}
		optimize "On" 
		links {"libprotobuf"}

	filter {"system:Windows"}
		defines {"_WINDOWS", "_USRDLL"}

	filter {"system:Windows", "configurations:Debug"}
		buildoptions {"-D_SCL_SECURE_NO_WARNINGS"}

