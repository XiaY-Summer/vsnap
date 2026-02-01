#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include <sstream>
#include <iomanip>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <bcrypt.h>
#else
#include <openssl/sha.h>
#endif

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

        bool ok = false;
#ifdef _WIN32
        BCRYPT_ALG_HANDLE hAlg = nullptr;
        BCRYPT_HASH_HANDLE hHash = nullptr;
        DWORD cbData = 0;
        DWORD cbHashObject = 0;
        DWORD cbHash = 0;
        if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) != 0) return "";
        if (BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&cbHashObject), sizeof(DWORD), &cbData, 0) != 0) {
            BCryptCloseAlgorithmProvider(hAlg, 0);
            return "";
        }
        if (BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, reinterpret_cast<PUCHAR>(&cbHash), sizeof(DWORD), &cbData, 0) != 0) {
            BCryptCloseAlgorithmProvider(hAlg, 0);
            return "";
        }
        std::vector<unsigned char> hashObject(cbHashObject);
        std::vector<unsigned char> hash(cbHash);
        if (BCryptCreateHash(hAlg, &hHash, hashObject.data(), cbHashObject, nullptr, 0, 0) != 0) {
            BCryptCloseAlgorithmProvider(hAlg, 0);
            return "";
        }
#else
        unsigned char hash[SHA256_DIGEST_LENGTH];
#endif

        if (size <= SMALL_FILE_THRESHOLD) {
            // 小文件：一次性读取
            std::string buffer(size, '\0');
            if (fs.read(&buffer[0], static_cast<std::streamsize>(size))) {
#ifdef _WIN32
                if (BCryptHashData(hHash, reinterpret_cast<PUCHAR>(buffer.data()),
                                   static_cast<ULONG>(size), 0) == 0) {
                    ok = (BCryptFinishHash(hHash, hash.data(), cbHash, 0) == 0);
                }
#else
                ok = (SHA256(reinterpret_cast<const unsigned char*>(buffer.data()),
                            size, hash) != nullptr);
#endif
            }
        } else {
            // 大文件：流式分块计算
#ifdef _WIN32
            std::vector<char> buf(STREAM_BUFFER_SIZE);
            bool hashOk = true;
            while (true) {
                fs.read(buf.data(), static_cast<std::streamsize>(buf.size()));
                size_t n = static_cast<size_t>(fs.gcount());
                if (n == 0) break;
                if (BCryptHashData(hHash, reinterpret_cast<PUCHAR>(buf.data()),
                                   static_cast<ULONG>(n), 0) != 0) {
                    hashOk = false;
                    break;
                }
            }
            if (hashOk) {
                ok = (BCryptFinishHash(hHash, hash.data(), cbHash, 0) == 0);
            }
#else
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
#endif
        }

#ifdef _WIN32
        if (hHash) BCryptDestroyHash(hHash);
        if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
#endif
        if (!ok) return "";

        // 转为十六进制字符串
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
#ifdef _WIN32
        for (size_t i = 0; i < hash.size(); ++i) {
            oss << std::setw(2) << static_cast<unsigned>(hash[i]);
        }
#else
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            oss << std::setw(2) << static_cast<unsigned>(hash[i]);
        }
#endif
        return oss.str();
    } catch (...) {
        return "";
    }
}
