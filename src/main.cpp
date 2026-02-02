#include <iostream>
#include <string>
#include <clocale>
#include <locale>
#ifdef _WIN32
#include <windows.h>
#endif
#include "option.hpp"
#include "log.hpp"

using namespace std;

//解析参数
bool parseArgs(int argc, char* argv[]){

    for (int i = 1; i < argc; i++) {
        //cout<<"参数[解析]: " << i << ": " << argv[i] << endl;
        if(string(argv[i]) == "--help" || string(argv[i]) == "-h") {
            config.mode = "help";
            return true;
        }

        else if(string(argv[i]) == "--init" || string(argv[i]) == "init") {
            config.mode = "init";
            config.major = 0;
            config.minor = 1;
            config.patch = 0;
            return true;
        }

        else if(string(argv[i]) == "--snap" || string(argv[i]) == "-s") {
            config.mode = "snap";
        }
        else if(string(argv[i]) == "--option" || string(argv[i]) == "-o") {
            config.option = argv[i + 1];
            // 检查option是否为空
            if(config.option.empty()) {
                vsnap_log::printError("选项不能为空！使用 --help 查看用法");
                return false;
            }
            i++;
        }
        else if(string(argv[i]) == "-t" || string(argv[i]) == "--tag") {
            config.tag = argv[i + 1];
            // 检查tag是否为空
            if(config.tag.empty()) {
                vsnap_log::printError("备注不能为空！使用 --help 查看用法");
                return false;
            }
            i++;
        }
        else if(string(argv[i]) == "--list" || string(argv[i]) == "-l") {
            config.mode = "list";
        }
        else if(string(argv[i]) == "--restore" || string(argv[i]) == "-r") {
            config.mode = "restore";
            config.restoreVersion = argv[i + 1];
            // 检查restoreVersion是否为空
            if(config.restoreVersion.empty()) {
                vsnap_log::printError("版本不能为空！使用 --help 查看用法");
                return false;
            }
            i++;
        }
        else if(string(argv[i]) == "--status") {
            config.mode = "status";
        }
        else return false;
    }
    
    return true;  // 添加返回值
}

// 判断模式，并且执行
void judgeMode() {
    if(config.mode == "help") {
        help();
        return;
    }
    if(config.mode == "init") {
        init();
    }
    else if(config.mode == "snap") {
        snap();
    }
    else if(config.mode == "list") {
        listSnapshots();
    }
    else if(config.mode == "restore") {
        restore();
    }
    else if(config.mode == "status") {
        status();
    }
    else {
        vsnap_log::printError("模式: " + config.mode);
        vsnap_log::printError("模式错误！使用 --help 查看用法");
        return;
    }
}
int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            bool vtOk = SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
            vsnap_log::setColorEnabled(vtOk);
        }
    }
    if (hErr != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hErr, &mode)) {
            SetConsoleMode(hErr, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
#endif
#ifdef _WIN32
    std::setlocale(LC_ALL, ".UTF-8");
#else
    std::setlocale(LC_ALL, "");
#endif
    try {
#ifdef _WIN32
        std::locale::global(std::locale(".UTF-8"));
#else
        std::locale::global(std::locale(""));
#endif
        std::cout.imbue(std::locale());
        std::cerr.imbue(std::locale());
    } catch (...) {
    }
    if(!parseArgs(argc, argv)) {
        vsnap_log::printError("参数错误！使用 --help 查看用法");
        return 0;
    }
    // 无参数或未指定模式时显示帮助
    if (argc == 1 || config.mode.empty()) {
        // 模拟 --help 输出
        help();
        return 0;
    }
    judgeMode();
    
    return 0;
}
