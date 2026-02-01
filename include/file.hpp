#pragma once

#include <string>
#include <filesystem>
#include <chrono>
#include <sys/stat.h>

using namespace std;

// 扫描当前目录所有文件 (递归) 排除.vsnap目录， string类型返回
string scanFiles(const string& path) {
    string file;
    for(const auto& entry : filesystem::directory_iterator(path)) {
        if(filesystem::is_directory(entry.path())) {
            if(entry.path().filename() == ".vsnap") {
                continue;
            }
            file += scanFiles(entry.path().string());
        }
        else {
            //cout<<"file: "<<entry.path().string()<<endl;
            file += entry.path().string() + "\n";
        }
    }
    return file;
}

// 获取文件大小，返回 string 类型
string getFileSize(const string& filePath) {
    try {
        auto size = filesystem::file_size(filePath);
        return to_string(size);
    } catch (...) {
        return "0";
    }
}

// 获取文件最后修改时间，返回 string 类型（秒级时间戳）
string getFileLastModifyTime(const string& filePath) {
    try {
        struct stat st;
        if (stat(filePath.c_str(), &st) != 0) return "0";
        return to_string(st.st_mtime);
    } catch (...) {
        return "0";
    }
}