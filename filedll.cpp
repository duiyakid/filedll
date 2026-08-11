#include "pch.h"
#include "filedll.h"


// 声明 fs 命名空间（C++17 filesystem 必须）
namespace fs = std::filesystem;

// ====================== 成员函数实现 ======================
// 构造函数：保持原有逻辑
CMD::CMD() {
    readCurrentDirFiles();
}

// 读取当前目录文件：增加异常捕获，避免崩溃
void CMD::readCurrentDirFiles() {
    filesVector.clear();  // 清空旧数据，避免重复
    // 遍历当前目录（非递归，仅当前层）
    try {
        for (const auto& entry : fs::directory_iterator(currentPath)) {
            // 只保留普通文件（跳过文件夹、符号链接等）
            if (fs::is_regular_file(entry)) {
                // 存入「仅文件名+后缀」（如 "photo.jpg"，不含路径）
                filesVector.push_back(entry.path().filename());
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "[读取目录失败] " << e.what() << std::endl;
    }
}

// 重命名函数：增加扩展名校验，返回错误码
int CMD::rename(std::string extension) {
    format = "-d \"%Y-%m-%d_%H-%M-%S_EXIF%%c.%%e\"";

    if (extension == "jpg") {
        fileExtension = "*.jpg";
        fileNameAssignment = "\"-FileName<${DateTimeOriginal}\"";
    }
    else if (extension == "mp4") {
        fileExtension = "*.mp4";
        format = "-api quicktimeUTC=1 -d \"%Y-%m-%d_%H-%M-%S_EXIF%%c.%%e\"";
        fileNameAssignment = "\"-FileName<${CreateDate}\"";
    }
    else if (extension == "png") {
        fileExtension = "*.png";
        fileNameAssignment = "\"-FileName<${createdate}\"";
        std::cout << "PNG格式文件重命名支持有限，请确保EXIF创建日期存在！" << std::endl;
    }
    else {
        std::cerr << "[不支持的扩展名] " << extension << std::endl;
        return -1; // 非0表示失败
    }
    std::string cmd = exiftool + " " + format + " " + fileNameAssignment + " " + fileExtension;
    int ret = system(cmd.c_str());
    return ret;
}

// 提取纯数字串：截取前14位，避免超长
std::vector<std::string> CMD::extractPureNumbers() {
    std::vector<std::string> pureNums;
    std::string currentNum;
    for (const auto& filename : filesVector) {
        currentNum.clear();
        // 遍历文件名的「主干」（不含后缀），提取纯数字

        for (char ch : filename.stem().string()) {
            unsigned char c = static_cast<unsigned char>(ch);
            if (isdigit(c)) {
                currentNum += c;
            }

        }
        if (!currentNum.empty()) {
            pureNums.push_back(currentNum);
        }
        // 找到14位数字串（YYYYMMDDHHMMSS），调用写入 EXIF
        if (currentNum.length() >= 14) {
            // 传入「完整文件路径」（当前目录 + 文件名）
            std::string fullFilePath = currentPath.string() + "\\" + filename.string();
            writeToExif(fullFilePath, currentNum.substr(0, 14), filename.extension().string()); // 截取前14位
        }
        else {
            std::cout << "[跳过] 文件名中未找到14位数字串，无法写入EXIF: " << filename.string() << std::endl;
        }
    }

    std::cout << std::endl;  // 换行，格式更整洁
    return pureNums;
}

// 新增：UTF-8 转宽字符（替代废弃的 wstring_convert）
std::wstring CMD::utf8ToWide(const std::string& utf8Str) {
    if (utf8Str.empty()) return {};

    // 获取所需宽字符长度（不含结束符）
    int wideLen = MultiByteToWideChar(
        CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), nullptr, 0
    );
    if (wideLen <= 0) {
        std::cerr << "[UTF-8转宽字符失败] " << utf8Str << " 错误码：" << GetLastError() << std::endl;
        return {};
    }

    std::wstring wideStr(wideLen, L'\0');
    MultiByteToWideChar(
        CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), &wideStr[0], wideLen
    );
    return wideStr;
}

// 修复：UTF-8 转 ANSI（适配 Windows API）
std::string CMD::utf8ToAnsi(const std::string& utf8Str) {
    // 第一步：UTF-8 → 宽字符串（自定义函数，无废弃接口）
    std::wstring wideStr = utf8ToWide(utf8Str);
    if (wideStr.empty()) {
        std::cerr << "[编码转换失败] UTF-8转宽字符失败，使用原路径" << std::endl;
        return utf8Str;
    }

    // 第二步：宽字符串 → ANSI（中文系统为 GBK，不含结束符）
    int ansiLen = WideCharToMultiByte(
        CP_ACP, 0, wideStr.c_str(), (int)wideStr.size(), nullptr, 0, nullptr, nullptr
    );
    if (ansiLen <= 0) {
        std::cerr << "[宽字符转ANSI失败] " << utf8Str << " 错误码：" << GetLastError() << std::endl;
        return utf8Str;
    }

    std::string ansiStr(ansiLen, 0);
    WideCharToMultiByte(
        CP_ACP, 0, wideStr.c_str(), (int)wideStr.size(), &ansiStr[0], ansiLen, nullptr, nullptr
    );
    return ansiStr;
}

// 修复后的短路径转换函数（适配中文路径）
std::string CMD::getShortPath(const std::string& longPathUtf8) {
    // 步骤1：UTF-8 转 ANSI（适配 Windows API）
    std::string longPathAnsi = utf8ToAnsi(longPathUtf8);
    if (longPathAnsi.empty()) {
        std::cerr << "[路径转换警告] 编码转换失败，使用原路径" << std::endl;
        return longPathUtf8;
    }

    // 步骤2：调用 Windows API 转换短路径
    char shortPathBuffer[MAX_PATH] = { 0 };
    DWORD result = GetShortPathNameA(
        longPathAnsi.c_str(), shortPathBuffer, MAX_PATH
    );

    if (result > 0 && result <= MAX_PATH) {
        return std::string(shortPathBuffer);
    }
    else {
        std::cerr << "[警告] 路径转换失败（错误码：" << GetLastError() << "），使用原路径：" << longPathUtf8 << std::endl;
        return longPathUtf8;
    }
}

// 核心：写入EXIF信息（一对一处理单个文件）
void CMD::writeToExif(std::string fullFilePath, std::string nums, std::string extension) {
    // 1. 格式化日期（YYYYMMDDHHMMSS → YYYY:MM:DD HH:MM:SS）
    std::string formattedDateTime = nums.substr(0, 4) + ":" +
        nums.substr(4, 2) + ":" +
        nums.substr(6, 2) + " " +
        nums.substr(8, 2) + ":" +
        nums.substr(10, 2) + ":" +
        nums.substr(12, 2);

    std::string modifyExif;
    std::string cmd;

    // 2. 按文件类型拼接命令
    if (extension == ".jpg") {
        modifyExif = "-overwrite_original -datetimeoriginal=\"" + formattedDateTime + "\"";
        std::string shortPath = getShortPath(fullFilePath);
        cmd = exiftool + " " + modifyExif + " \"" + shortPath + "\"";
    }
    else if (extension == ".mp4") {
        modifyExif = "-overwrite_original -api QuickTimeUTC=1 -CreateDate=\"" + formattedDateTime + "+08:00\"";
        std::string shortPath = getShortPath(fullFilePath); // MP4也转短路径，避免中文问题
        cmd = exiftool + " " + modifyExif + " \"" + shortPath + "\"";
    }
    else if (extension == ".png") {
        modifyExif = "-overwrite_original -createdate=\"" + formattedDateTime + "\"";
        std::string shortPath = getShortPath(fullFilePath);
        cmd = exiftool + " " + modifyExif + " \"" + shortPath + "\"";
    }
    else {
        std::cerr << "[不支持的文件格式] " << extension << " → 跳过文件：" << fullFilePath << std::endl;
        return;
    }

    // 3. 执行命令并输出结果
    int ret = system(cmd.c_str());

    // 4. 错误判断
    if (ret != 0) {
        std::cerr << "错误：执行 exiftool 命令失败！（错误码：" << ret << "）文件：" << fullFilePath << std::endl;
    }

}