#pragma once

namespace Kans
{
	class FileSystem
	{
	public:
		static std::string GetAssetPath(const std::string& path);
		static std::string GetShaderPath(const std::string& path);
	};
}