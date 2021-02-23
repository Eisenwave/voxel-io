#include "voxelio/filetype.hpp"

#include "voxelio/stringmanip.hpp"

#include <cstring>
#include <iostream>
#include <unordered_map>

namespace voxelio {

static std::unordered_map<std::string, FileType> buildSuffixMap()
{
    std::unordered_map<std::string, FileType> result;

    for (FileType type : FILE_TYPE_VALUES) {
        result.emplace(extensionOf(type), type);
        if (auto altExtension = alternativeExtensionOf(type); std::strlen(altExtension) != 0) {
            result.emplace(altExtension, type);
        }
    }
    return result;
}

static const auto SUFFIX_MAP = buildSuffixMap();

std::optional<FileType> fileTypeOfExtension(const std::string &lowerCaseSuffix)
{
    auto type = SUFFIX_MAP.find(lowerCaseSuffix);
    return type == SUFFIX_MAP.end() ? std::nullopt : std::optional<FileType>{type->second};
}

std::optional<FileType> detectFileType(const std::string &path)
{
    auto result = detectFileTypeUsingName(path);
    return result.has_value() ? result : detectFileTypeUsingMagic(path);
}

std::optional<FileType> detectFileTypeUsingName(const std::string &path)
{
    auto separatorIndex = path.rfind('.');
    if (separatorIndex == std::string::npos) {
        return std::nullopt;
    }

    std::string lowerCaseSuffix = path.substr(separatorIndex + 1);
    toLowerCase(lowerCaseSuffix);

    return fileTypeOfExtension(lowerCaseSuffix);
}

std::optional<FileType> detectFileTypeUsingMagic(const std::string &)
{
    // TODO implement
    return std::nullopt;
}

std::ostream &operator<<(std::ostream &stream, FileType fileType)
{
    return stream << nameOf(fileType);
}

}  // namespace voxelio
