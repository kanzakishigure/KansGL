#include "kspch.h"
#include "FileSystem.h"
namespace Kans 
{
	std::string FileSystem::GetAssetPath(const std::string& path)
	{
		return "asset/" + path;
	}

	std::string FileSystem::GetShaderPath(const std::string& path)
	{
		return "asset/shader/" + path;
	}
}
