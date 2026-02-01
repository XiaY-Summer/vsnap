#pragma once

#include <string>
#include <filesystem>
#include <chrono>

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
            file += entry.path().generic_string() + "\n";
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
        auto ftime = filesystem::last_write_time(filePath);
        auto sctp = chrono::time_point_cast<chrono::system_clock::duration>(
            ftime - filesystem::file_time_type::clock::now() + chrono::system_clock::now()
        );
        return to_string(chrono::system_clock::to_time_t(sctp));
    } catch (...) {
        return "0";
    }
}