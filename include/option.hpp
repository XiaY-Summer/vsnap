#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <set>
#include <algorithm>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "file.hpp"
#include "hash.hpp"
#include "time.hpp"
#include "log.hpp"

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

using namespace std;

// 配置结构体
struct config {
    string mode;
    string tag;
    string option;
    string restoreVersion;
    int major;
    int minor;
    int patch;
};
struct fileInfo{
    string filePath;
    string fileName;
    string fileSize;
    string fileHash;
    string fileLastModifyTime;
};

inline config config;
inline fileInfo fileInfo;

// 命令执行类
class command {
public:
    void mkdir(const std::string& dirName) {
        try {
            std::filesystem::create_directories(std::filesystem::path(dirName));
        } catch (...) {
            // ignore
        }
    }

    void touch(const std::string& fileName) {
        std::ofstream(fileName, std::ios::app).close();
    }

    // 复制文件
    void copy(const std::string& srcFile, const std::string& destFile) {
        try {
            std::filesystem::path destPath(destFile);
            if (destPath.has_parent_path()) {
                std::filesystem::create_directories(destPath.parent_path());
            }
            std::filesystem::copy_file(
                std::filesystem::path(srcFile),
                destPath,
                std::filesystem::copy_options::overwrite_existing
            );
        } catch (...) {
            // ignore
        }
    }
    // 列出目录下所有文件（使用 filesystem，返回空格分隔的文件名）
    string ls(const std::string& dirName) {
        std::string result;
        try {
            for (const auto& entry : std::filesystem::directory_iterator(dirName)) {
                if (!result.empty()) result += " ";
                result += entry.path().filename().string();
            }
        } catch (...) {
            return "";
        }
        return result;
    }

    // 删除文件
    void rm(const std::string& fileName) {
        try {
            std::filesystem::remove_all(std::filesystem::path(fileName));
        } catch (...) {
            // ignore
        }
    }

    bool exists(const std::string& fileName) {
        try {
            return std::filesystem::exists(std::filesystem::path(fileName));
        } catch (...) {
            return false;
        }
    }
};

inline bool help() {
    cout << endl;
    cout << vsnap_log::color(vsnap_log::ansi::bold) << "  vsnap" << vsnap_log::reset() << vsnap_log::color(vsnap_log::ansi::dim) << " - 轻量级版本快照工具" << vsnap_log::reset() << endl << endl;
    vsnap_log::info() << "用法:" << endl;
    vsnap_log::bullet() << "vsnap -h, --help" << vsnap_log::color(vsnap_log::ansi::dim) << "                  显示此帮助" << vsnap_log::reset() << endl;
    vsnap_log::bullet() << "vsnap --init" << vsnap_log::color(vsnap_log::ansi::dim) << "                      初始化仓库（创建 .vsnap 并生成 0.1.0 快照）" << vsnap_log::reset() << endl;
    vsnap_log::bullet() << "vsnap --list" << vsnap_log::color(vsnap_log::ansi::dim) << "                      列出所有快照（版本、备注、创建时间）" << vsnap_log::reset() << endl;
    vsnap_log::bullet() << "vsnap --show" << vsnap_log::color(vsnap_log::ansi::dim) << "                      显示当前版本号" << vsnap_log::reset() << endl;
    vsnap_log::bullet() << "vsnap --snap" << vsnap_log::color(vsnap_log::ansi::dim) << "                      创建新快照（默认 patch 递增）" << vsnap_log::reset() << endl;
    vsnap_log::bullet() << "    [--option patch|minor|major]" << vsnap_log::color(vsnap_log::ansi::dim) << "  版本递增方式：patch/minor/major" << vsnap_log::reset() << endl;
    vsnap_log::bullet() << "    [-t, --tag \"备注\"]" << vsnap_log::color(vsnap_log::ansi::dim) << "            为本次快照添加备注" << vsnap_log::reset() << endl;
    vsnap_log::bullet() << "vsnap -r, --restore <版本号>" << vsnap_log::color(vsnap_log::ansi::dim) << "      将工作区还原到指定版本（如 1.0.0）" << vsnap_log::reset() << endl;
    cout << endl;
    return true;
}

inline bool snap(string mode = "patch") {
    if (!config.option.empty()) {
        mode = config.option;
    }

    string fileList = scanFiles(".");
    command cmd;
    if (!cmd.exists(".vsnap")) {
        vsnap_log::printError("未初始化！请先执行 vsnap --init");
        return false;
    }
    ifstream configIn(".vsnap/config.json");
    if (!configIn.is_open()) {
        vsnap_log::printError("无法打开 .vsnap/config.json");
        return false;
    }
    json repoConfig;
    configIn >> repoConfig;
    configIn.close();
    config.major = repoConfig["current_version"]["major"].get<int>();
    config.minor = repoConfig["current_version"]["minor"].get<int>();
    config.patch = repoConfig["current_version"]["patch"].get<int>();

    if (mode == "patch") {
        config.patch++;
    } else if (mode == "minor") {
        config.minor++;
        config.patch = 0;
    } else if (mode == "major") {
        config.major++;
        config.minor = 0;
        config.patch = 0;
    } else {
        vsnap_log::printError("模式错误！仅支持 patch, minor, major");
        return false;
    }

    repoConfig["current_version"]["patch"] = config.patch;
    repoConfig["current_version"]["minor"] = config.minor;
    repoConfig["current_version"]["major"] = config.major;
    ofstream configOut(".vsnap/config.json");
    configOut << repoConfig.dump(4);
    configOut.close();

    string Version = to_string(config.major) + "." + to_string(config.minor) + "." + to_string(config.patch);

    // 输出fileList到文件fileList.txt
    //ofstream fileListOut("fileList.txt");
    //fileListOut << fileList;
    //fileListOut.close();

    stringstream ss(fileList);
    string line;
    ordered_json fileInfoArray = ordered_json::array();
    // 在json文件的最开头加上版本号
    fileInfoArray.push_back({
        {"version", Version}
    });
    fileInfoArray.push_back({
        {"tag", config.tag}
    });
    fileInfoArray.push_back({
        {"created_at", getCurrentTime()}
    });
    
    while (getline(ss, line)) {
        if (line.find(".vsnap") != string::npos) {
            continue;
        }
        if (!line.empty()) {
            // 使用fileInfo结构体保存文件信息
            fileInfo.filePath = line;
            fileInfo.fileName = line.substr(line.find_last_of("/") + 1);
            fileInfo.fileSize = getFileSize(line);
            fileInfo.fileLastModifyTime = getFileLastModifyTime(line);
            fileInfo.fileHash = getFileHash(line);

            fileInfoArray.push_back({
                {"fileName", fileInfo.fileName},
                {"Path", fileInfo.filePath},
                {"Size", fileInfo.fileSize},
                {"LastModifyTime", fileInfo.fileLastModifyTime},
                {"Hash", fileInfo.fileHash}
            });
        }
    }
    string json_fileInfo = fileInfoArray.dump(4);
    string fileInfoVersionJsonPath = ".vsnap/snapshots/" + Version + ".json";

    ofstream snapshotOut(fileInfoVersionJsonPath);
    if (!snapshotOut.is_open()) {
        vsnap_log::printError("无法创建快照文件: " + fileInfoVersionJsonPath);
        return false;
    }
    snapshotOut << json_fileInfo;
    snapshotOut.close();

    vsnap_log::success() << "配置已创建" << endl;
    vsnap_log::info() << "版本: " << config.major << "." << config.minor << "." << config.patch << endl;

    ss.clear();
    ss.str(fileList);
    while (getline(ss, line)) {
        if (!line.empty()) {
            if (line.find(".vsnap") == string::npos) {
                if (cmd.exists(".vsnap/objects/" + getFileHash(line) + ".bin")) {
                    vsnap_log::printWarn("文件未变更，跳过: " + line);
                    continue;
                }
                cmd.copy(line, ".vsnap/objects/" + getFileHash(line) + ".bin");
            }
        }
    }

    vsnap_log::printSuccess("快照创建成功！");
    return true;
}

inline bool init() {
    command cmd;
    if (cmd.exists(".vsnap")) {
        vsnap_log::printWarn("已初始化！当前目录已是 vsnap 仓库");
        return false;
    }
    cmd.mkdir(".vsnap");
    cmd.touch(".vsnap/config.json");
    cmd.mkdir(".vsnap/snapshots");
    cmd.mkdir(".vsnap/objects");
    vsnap_log::printSuccess("仓库初始化成功！");
    ordered_json cfg = {
        {"version", "0.1.0"},
        {"current_version", {
            {"major", 0},
            {"minor", 0},
            {"patch", 0}
        }},
        {"created_at", getCurrentTime()}
    };
    ofstream fout(".vsnap/config.json");
    fout << cfg.dump(4);
    fout.close();
    
    snap("minor");
    return true;
}

inline bool listSnapshots() {
    command command;
    if (!command.exists(".vsnap")) {
        vsnap_log::printError("未初始化！请先执行 vsnap --init");
        return false;
    }
    string list = command.ls(".vsnap/snapshots");
    if (list.empty()) {
        vsnap_log::printInfo("暂无快照");
        return true;
    }
    // 收集快照数据
    struct SnapshotRow {
        string version;
        string tag;
        string created_at;
    };
    vector<SnapshotRow> rows;
    stringstream ss(list);
    string version;
    while (getline(ss, version, ' ')) {
        if (version.empty()) continue;
        SnapshotRow row;
        row.version = version.substr(0, version.find(".json"));
        ifstream jsonFile(".vsnap/snapshots/" + version);
        if (!jsonFile.is_open()) {
            vsnap_log::printWarn("无法打开快照文件: " + version);
            continue;
        }
        json jsonData;
        jsonFile >> jsonData;
        jsonFile.close();
        row.tag = (jsonData.size() > 1 && jsonData[1].contains("tag")) ? jsonData[1]["tag"].get<string>() : "";
        row.created_at = (jsonData.size() > 2 && jsonData[2].contains("created_at")) ? jsonData[2]["created_at"].get<string>() : "";
        rows.push_back(row);
    }
    if (rows.empty()) return true;
    // UTF-8 字符串的终端显示宽度（中文=2列，ASCII=1列）
    auto displayWidth = [](const string& s) -> size_t {
        size_t w = 0;
        for (size_t i = 0; i < s.length(); ) {
            unsigned char c = (unsigned char)s[i];
            if (c < 0x80) {
                w += 1;
                i += 1;
            } else if ((c & 0xE0) == 0xC0) {
                w += 2;
                i += 2;
            } else if ((c & 0xF0) == 0xE0) {
                w += 2;  // 中文等 3 字节字符
                i += 3;
            } else if ((c & 0xF8) == 0xF0) {
                w += 2;
                i += 4;
            } else {
                i += 1;  // 续字节或无效，跳过
            }
        }
        return w;
    };
    // 按显示宽度右填充空格
    auto padRight = [&displayWidth](const string& s, size_t targetW) {
        size_t w = displayWidth(s);
        return s + string(targetW > w ? targetW - w : 0, ' ');
    };
    size_t wVersion = 6, wtag = 6, wCreated = 10;
    for (const auto& r : rows) {
        wVersion = max(wVersion, displayWidth(r.version));
        wtag = max(wtag, displayWidth(r.tag));
        wCreated = max(wCreated, displayWidth(r.created_at));
    }
    wVersion = max(wVersion, displayWidth("版本"));
    wtag = max(wtag, displayWidth("备注"));
    wCreated = max(wCreated, displayWidth("创建时间"));
    // 按版本号倒序（最新在前）
    auto parseVersion = [](const string& v) {
        int maj = 0, min = 0, pat = 0;
        sscanf(v.c_str(), "%d.%d.%d", &maj, &min, &pat);
        return make_tuple(maj, min, pat);
    };
    sort(rows.begin(), rows.end(), [&parseVersion](const SnapshotRow& a, const SnapshotRow& b) {
        return parseVersion(a.version) > parseVersion(b.version);
    });
    const string sep = " | ";
    cout << vsnap_log::color(vsnap_log::ansi::bold);
    cout << "| " << padRight("版本", wVersion) << sep << padRight("备注", wtag) << sep << padRight("创建时间", wCreated) << " |" << endl;
    cout << vsnap_log::reset();
    cout << "|" << string(wVersion + 2, '-') << "|" << string(wtag + 2, '-') << "|" << string(wCreated + 2, '-') << "|" << endl;
    for (const auto& r : rows) {
        cout << "| " << padRight(r.version, wVersion) << sep << padRight(r.tag, wtag) << sep << padRight(r.created_at, wCreated) << " |" << endl;
    }
    return true;
}

inline bool restore() {
    command command;
    // 校验.vsnap目录是否存在
    if (!command.exists(".vsnap")) {
        vsnap_log::printError("未初始化！请先执行 vsnap --init");
        return false;
    }
    string version = config.restoreVersion;
    string jsonFilePath = ".vsnap/snapshots/" + version + ".json";
    // 校验对应版本的快照JSON文件是否存在
    if (!command.exists(jsonFilePath)) {
        vsnap_log::printError("版本不存在！请先执行 vsnap --list 查看版本");
        return false;
    }


    // load
    ifstream jsonFile(jsonFilePath);
    if (!jsonFile.is_open()) {
        vsnap_log::printError("无法打开快照文件: " + jsonFilePath);
        return false;
    }
    json jsonData;
    try {
        jsonFile >> jsonData;
    } catch (const json::exception& e) {
        vsnap_log::printError("解析快照 JSON 失败: " + string(e.what()));
        jsonFile.close();
        return false;
    }
    jsonFile.close();
    // 检查JSON格式和有效条目个数
    if (!jsonData.is_array() || jsonData.size() <= 3) {
        vsnap_log::printError("快照中无有效文件信息");
        return false;
    }
    // 输出快照元信息
    vsnap_log::info() << "退回到版本: " << version << endl;
    vsnap_log::info() << "备注信息: " << jsonData[1].value("tag", string("无")) << endl;
    vsnap_log::info() << "创建时间: " << jsonData[2].value("created_at", string("未知")) << endl;
    cout << endl;

    // 收集快照中的路径（用于判断“不在快照中”的当前文件）
    set<string> snapshotPaths;
    for (size_t i = 3; i < jsonData.size(); ++i) {
        const auto& item = jsonData[i];
        if (!item.is_object()) continue;
        string p = item.value("Path", string(""));
        if (!p.empty() && p.find(".vsnap") == string::npos)
            snapshotPaths.insert(p);
    }

    // 找出当前目录中存在但不在快照中的文件，逐项询问删除/保留
    string currentFileList = scanFiles(".");
    vector<string> extraPaths;
    {
        istringstream iss(currentFileList);
        string line;
        while (getline(iss, line)) {
            if (line.empty() || line.find(".vsnap") != string::npos) continue;
            if (snapshotPaths.find(line) == snapshotPaths.end())
                extraPaths.push_back(line);
        }
    }
    if (!extraPaths.empty()) {
        vsnap_log::info() << "以下文件不在快照中，回退后默认会保留。可选择性删除：" << endl;
        bool deleteAll = false, keepAll = false;
        for (const string& path : extraPaths) {
            if (!command.exists(path)) continue;
            if (keepAll) continue;
            if (deleteAll) {
                command.rm(path);
                continue;
            }
            cout << "  " << path << "  删除(Y)/保留(N)/全删除(A)/全保留(M): ";
            string line;
            getline(cin, line);
            char choice = line.empty() ? 'N' : static_cast<char>(line[0]);
            if (choice == 'Y' || choice == 'y') {
                command.rm(path);
            } else if (choice == 'A' || choice == 'a') {
                deleteAll = true;
                command.rm(path);
            } else if (choice == 'M' || choice == 'm') {
                keepAll = true;
            }
        }
        cout << endl;
    }

    // 删除当前版本的所有文件，并排除.vsnap目录
    for (size_t i = 3; i < jsonData.size(); ++i) {
        const auto& item = jsonData[i];
        if (!item.is_object()) continue;
        fileInfo.fileName = item.value("fileName", string(""));
        fileInfo.filePath = item.value("Path", string(""));
        if (fileInfo.filePath.find(".vsnap") == string::npos) {
            command.rm(fileInfo.filePath);
        }
    }

    // 从第4项开始逐个读取文件信息
    for (size_t i = 3; i < jsonData.size(); ++i) {
        const auto& item = jsonData[i];
        if (!item.is_object()) continue;
        // 读取文件信息到结构体
        fileInfo.fileName           = item.value("fileName", string(""));
        fileInfo.filePath           = item.value("Path", string(""));
        fileInfo.fileSize           = item.value("Size", string(""));
        fileInfo.fileLastModifyTime = item.value("LastModifyTime", string(""));
        fileInfo.fileHash           = item.value("Hash", string(""));
        // 至少存在filePath或fileName之一才处理
        if (fileInfo.filePath.empty() && fileInfo.fileName.empty()) {
            continue;
        }
        // 输出文件信息
        if (vsnap_log::logLevel >= vsnap_log::DETAIL) {
            cout << vsnap_log::color(vsnap_log::ansi::bold) << "[" << (i - 2) << "] "
                 << vsnap_log::reset() << fileInfo.fileName << endl;
            vsnap_log::bullet() << "路径: " << fileInfo.filePath << endl;
            vsnap_log::bullet() << "大小: " << fileInfo.fileSize << " bytes" << endl;
            vsnap_log::bullet() << "修改时间: " << fileInfo.fileLastModifyTime << endl;
            vsnap_log::bullet() << "哈希: " << vsnap_log::color(vsnap_log::ansi::dim)
                                << fileInfo.fileHash << vsnap_log::reset() << endl;
        }
        
        // 例如：检查对象文件是否存在
        string objectPath = ".vsnap/objects/" + fileInfo.fileHash + ".bin";
        if (!command.exists(objectPath)) {
            vsnap_log::printError("对象文件缺失，跳过: " + fileInfo.filePath);
            continue;
        }

        // 判断文件大小和哈希是否一致
        if (fileInfo.fileSize != getFileSize(objectPath) || fileInfo.fileHash != getFileHash(objectPath)) {
            vsnap_log::printError("文件大小或哈希不一致，跳过: " + fileInfo.filePath);
            continue;
        }
        // 复制文件到目标位置
        command.copy(objectPath, fileInfo.filePath);
        
        vsnap_log::printSuccess("[" + to_string(i - 2) + "] " + fileInfo.fileName + " (" + fileInfo.fileSize + " bytes)");
    }
    // 输出总文件数
    vsnap_log::printInfo("总文件数: " + to_string(jsonData.size() - 3));
    // 修改 config.json 中的当前版本号为恢复的版本号
    ifstream configIn(".vsnap/config.json");
    if (configIn.is_open()) {
        json repoConfig;
        configIn >> repoConfig;
        configIn.close();
        int maj = 0, min = 0, pat = 0;
        sscanf(version.c_str(), "%d.%d.%d", &maj, &min, &pat);
        repoConfig["current_version"]["major"] = maj;
        repoConfig["current_version"]["minor"] = min;
        repoConfig["current_version"]["patch"] = pat;
        ofstream configOut(".vsnap/config.json");
        configOut << repoConfig.dump(4);
        configOut.close();
    }
    vsnap_log::printSuccess("版本还原完成！");
    return true;
}

inline bool show() {
    command command;
    if (!command.exists(".vsnap")) {
        vsnap_log::printError("未初始化！请先执行 vsnap --init");
        return false;
    }
    // 获取当前config.json中的当前版本号
    ifstream configIn(".vsnap/config.json");
    if (!configIn.is_open()) {
        vsnap_log::printError("无法打开 .vsnap/config.json");
        return false;
    }
    json configData;
    configIn >> configData;
    configIn.close();
    vsnap_log::printInfo("当前版本号: " + to_string(configData["current_version"]["major"].get<int>()) + "." + to_string(configData["current_version"]["minor"].get<int>()) + "." + to_string(configData["current_version"]["patch"].get<int>()));
    return true;
}
