#include <iostream>
#include <string>
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

        else if(string(argv[i]) == "--snap") {
            config.mode = "snap";
        }
        else if(string(argv[i]) == "--option") {
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
        else if(string(argv[i]) == "--list") {
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
        else if(string(argv[i]) == "--show") {
            config.mode = "show";
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
        list();
    }
    else if(config.mode == "restore") {
        restore();
    }
    else if(config.mode == "show") {
        show();
    }
    else {
        vsnap_log::printError("模式: " + config.mode);
        vsnap_log::printError("模式错误！使用 --help 查看用法");
        return;
    }
}
int main(int argc, char* argv[]) {
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
