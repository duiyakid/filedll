#pragma once
#ifndef FILEDLL_H
#define FILEDLL_H
#include <string>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
namespace fs = std::filesystem;
#ifdef FILEDLL_EXPORTS
#define FILEDLL_API __declspec(dllexport)
#else
#define FILEDLL_API __declspec(dllimport)
#endif
namespace FileDLL {

    FILEDLL_API void readCurrentDirFiles(std::vector<fs::path>& file_vector);
    FILEDLL_API std::vector<std::string> extractPureNumbers(const std::vector<fs::path>& file_vector);
    FILEDLL_API std::string getShortPath(const std::string& longPath);
    FILEDLL_API std::string utf8ToAnsi(const std::string& utf8Str);
    FILEDLL_API std::wstring utf8ToWide(const std::string& utf8Str);
}

#endif // !FILEDLL_H
