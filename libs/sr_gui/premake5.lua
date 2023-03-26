-- On Linux we have to query the dependencies for libsecret
if os.ishost("linux") then
	libFlags, code = os.outputof("pkg-config --cflags libnotify gtk+-3.0")
end

project("sr_gui")
	kind("StaticLib")
	systemversion("latest")
	
	filter("toolset:not msc*")
		buildoptions({ "-Wall", "-Wextra" })
	filter("toolset:msc*")
		buildoptions({ "-W3"})
	filter({})

	-- common files
	files({"sr_gui.h", "sr_gui_internal.h"})
	
	-- system build filters
	filter("system:windows")
		language("C++")
		files({"sr_gui_win.cpp"})

	filter("system:macosx")
		language("C")
		files({"sr_gui_mac.m"})

	filter({"system:linux"})
		language("C")
		buildoptions(libFlags)
		files({"sr_gui_lin.c"})

   
	filter({})

