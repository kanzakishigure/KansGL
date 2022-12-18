--include 文件夹 采用相对路径
IncludeDir ={}
IncludeDir["GLFW"] = "KansGL/vendor/GLFW/include"
IncludeDir["Glad"] = "KansGL/vendor/Glad/include"
IncludeDir["ImGui"] = "KansGL/vendor/imgui"
IncludeDir["GLM"] = "KansGL/vendor/glm"
IncludeDir["stb_image"] = "KansGL/vendor/stb_image"
IncludeDir["assimp"] = "KansGL/vendor/assimp/include"


LibraryDir = {}


Library = {}
Library["Assimp_Debug"] = "%{_WORKING_DIR}/KansGL/vendor/assimp/bin/Debug/assimp-vc142-mtd.lib"
Library["Assimp_Release"] = "%{_WORKING_DIR}/KansGL/vendor/assimp/bin/Release/assimp-vc142-mt.lib"


Binaries = {}
Binaries["Assimp_Debug"] = "%{_WORKING_DIR}/KansGL/vendor/assimp/bin/Debug/assimp-vc142-mtd.dll"
Binaries["Assimp_Release"] = "%{_WORKING_DIR}/KansGL/vendor/assimp/bin/Release/assimp-vc142-mt.dll"