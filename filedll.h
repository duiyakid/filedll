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

    // ====================== 路径解析工具 ======================

    // 1. 当前 exe 所在目录
	FILEDLL_API fs::path exe_path();
    FILEDLL_API fs::path exe_dir();

    // 2. 目录（父路径）
    FILEDLL_API fs::path path_dir(const fs::path& p);

    // 2b. 相对当前 exe 的目录
    FILEDLL_API fs::path path_dir_relative_exe(const fs::path& p);

    // 3. 文件名（带扩展，如 "file.tar.gz"）
    FILEDLL_API fs::path path_filename(const fs::path& p);

    // 4. 真文件名（主名，去所有扩展名，如 "file"）
    FILEDLL_API std::string path_basename(const fs::path& p);

    // 5a. 最后一个扩展名（如 ".gz"）
    FILEDLL_API std::string path_ext(const fs::path& p);

    // 5b. 全部扩展名（如 ".tar.gz"）
    FILEDLL_API std::string path_all_ext(const fs::path& p);

    // 6. 完整绝对路径
    FILEDLL_API fs::path     full_path(const fs::path& p);
    FILEDLL_API std::string  full_path_str(const fs::path& p);

    // ====================== 目录 / 文件操作 ======================

    // 读取当前目录下的普通文件（仅文件名，不含路径）
    FILEDLL_API void list_cwd_files(std::vector<fs::path>& file_vector);

    // 从文件名中提取纯数字串（前 14 位用于 EXIF）
    FILEDLL_API std::vector<std::string> digits_from_files(
        const std::vector<fs::path>& file_vector);

    // ====================== 编码转换 ======================

    // UTF-8 → 宽字符（替代废弃的 wstring_convert）
    FILEDLL_API std::wstring utf8ToWide(const std::string& utf8Str);

    // UTF-8 → ANSI（中文系统为 GBK）
    FILEDLL_API std::string utf8ToAnsi(const std::string& utf8Str);

    // 长路径 → 短路径（8.3 格式），中文路径可用
    FILEDLL_API std::string getShortPath(const std::string& longPathUtf8);
}

#endif // !FILEDLL_H