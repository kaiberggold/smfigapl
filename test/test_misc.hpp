#pragma once

#include <algorithm>
#include <vector>
#include <filesystem>
#include <string>

// Folder of the configuration files used by the tests (set by CMake)
inline const std::string testDataDir = SM_TEST_DATA_DIR;

inline std::vector<std::string> folderList(const std::string& path)
{
    std::vector<std::string> temp;
    temp.resize(0);

//workaround for not compiling with clang 10.0.1
    for (const auto& p : std::filesystem::directory_iterator(path))
    {
        if (p.path().extension().string() == ".yaml" && p.path().filename().string().substr(0, 1) == "0")
            temp.push_back(p.path().string());

    }
    std::ranges::sort(temp); // iterator is not ordered
    return temp;
}
