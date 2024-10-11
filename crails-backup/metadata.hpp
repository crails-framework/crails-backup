#pragma once
#include <filesystem>
#include <map>
#include <string>

typedef std::map<std::string,std::string> Metadata;

Metadata read_metadata(const std::filesystem::path& path);
