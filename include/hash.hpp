#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <openssl/sha.h>
#include <filesystem>
#include <sstream>
#include <iomanip>

/**
 * 计算文件 SHA-256 哈希值
 *
 * 针对「大量小文件 + 少数大文件」场景优化：
 * - 小文件（< 1MB）：一次性读入内存计算，减少系统调用
 * - 大文件：分块流式读取（64KB 缓冲区），避免内存溢出
 *
 * @param filePath 文件路径
 * @return 十六进制哈希字符串，失败返回空字符串
 */
inline std::string getFileHash(const std::string& filePath) {
    constexpr size_t SMALL_FILE_THRESHOLD = 1024 * 1024;  // 1MB
    constexpr size_t STREAM_BUFFER_SIZE = 64 * 1024;       // 64KB

    try {
        auto size = std::filesystem::file_size(filePath);
        std::ifstream fs(filePath, std::ios::binary);
        if (!fs) return "";

        unsigned char hash[SHA256_DIGEST_LENGTH];
        bool ok = false;

        if (size <= SMALL_FILE_THRESHOLD) {
            // 小文件：一次性读取
            std::string buffer(size, '\0');
            if (fs.read(&buffer[0], static_cast<std::streamsize>(size))) {
                ok = (SHA256(reinterpret_cast<const unsigned char*>(buffer.data()),
                            size, hash) != nullptr);
            }
        } else {
            // 大文件：流式分块计算
            SHA256_CTX ctx;
            if (SHA256_Init(&ctx)) {
                std::vector<char> buf(STREAM_BUFFER_SIZE);
                while (true) {
                    fs.read(buf.data(), static_cast<std::streamsize>(buf.size()));
                    size_t n = static_cast<size_t>(fs.gcount());
                    if (n == 0) break;
                    if (!SHA256_Update(&ctx, buf.data(), n)) break;
                }
                ok = SHA256_Final(hash, &ctx);
            }
        }

        if (!ok) return "";

        // 转为十六进制字符串
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            oss << std::setw(2) << static_cast<unsigned>(hash[i]);
        }
        return oss.str();
    } catch (...) {
        return "";
    }
}
