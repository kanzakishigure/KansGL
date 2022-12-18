include "dependencies.lua"
workspace "KansGL"
    architecture"x64"
    startproject "KansGL"

    configurations 
    { 
        "Debug", 
        "Release",
        "Dist" 
    }
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


group"Dependencies"
include "KansGL/vendor/GLFW"
include "KansGL/vendor/Glad"
include "KansGL/vendor/imgui"
group""

project "KansGL"    
    location"KansGL"
    kind "ConsoleApp"
    language "C++"
    cppdialect"c++17"
    staticruntime"off"


    targetdir ("bin/" ..outputdir.. "/%{prj.name}")
    objdir ("bin-int/" ..outputdir.. "/%{prj.name}")
    pchheader "kspch.h"
    pchsource "KansGL/src/kspch.cpp"
    files 
    { 
        "%{prj.name}/Main.cpp",
        "%{prj.name}/src/**.h", 
        "%{prj.name}/src/**.cpp" ,
        "%{prj.name}/vendor/stb_image/**.h", 
        "%{prj.name}/vendor/stb_image/**.cpp",
        "%{prj.name}/vendor/glm/glm/**.hpp",
        "%{prj.name}/vendor/glm/glm/**.inl"
        
    }
    includedirs 
    { 
        
        "%{prj.name}/src",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.GLM}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.assimp}"
        
        
    }
    links
    {
        "GLFW",
        "Glad",
        "ImGui",
        "opengl32.lib"
    }
    filter "system:windows" 
        systemversion "latest" 

        defines
        {
            "HZ_PLATFORM_WINDOWS",
            "HZ_BUILD_DLL",
            "GLFW_INCLUDE_NONE",
            "_CRT_SECURE_NO_WARNINGS"
        }

    filter "configurations:Debug"      
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
    filter "configurations:Dist"
        runtime "Release"
        optimize "on"


