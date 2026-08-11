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


class CMD {
private:
    fs::path currentPath = fs::current_path();
    std::vector<fs::path> filesVector;
    std::string exiftool = "exiftool -overwrite_original";
    std::string format = "-d \"%Y-%m-%d_%H-%M-%S_EXIF.%%e\"";
    std::string fileNameAssignment = "\"-FileName<${DateTimeOriginal}\"";
    std::string fileExtension = "*.jpg";// *.mp4
public:
    CMD();
    std::string getExiftool() const { return exiftool; }
    std::string getFormat() const { return format; }
    std::string getFileNameAssignment() const { return fileNameAssignment; }
    std::string getFileExtension() const { return fileExtension; }
    int rename(std::string extension);
    void readCurrentDirFiles();
    void writeToExif(std::string filename, std::string nums, std::string extension);
    std::vector<std::string> extractPureNumbers();
    std::string getShortPath(const std::string& longPath);
    std::string utf8ToAnsi(const std::string& utf8Str);
    std::wstring utf8ToWide(const std::string& utf8Str);

};

#endif // !FILEDLL_H
