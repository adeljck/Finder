#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <windows.h>
#include <string>
#include <vector>
#include <iostream>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include <locale>
#include <fstream>
#include <cwctype>
#include "../include/app.h"
#include "../include/config.h"
#include "../include/scanner.h"
#include "../include/utils.h"
#include "../include/output.h"

using wstring = std::wstring; using string = std::string;

int run_app(int argc, wchar_t **argv) {
    std::locale::global(std::locale(""));
    std::vector<wstring> paths;
    Config cfg = parse_args(argc, argv, paths);
    if (paths.empty() && !cfg.scan_all) {
        std::wcout << L"用法: finder.exe [选项] [路径1 路径2 ...]\n"
                    L"  -all                 扫描所有磁盘\n"
                    L"  -k kw1,kw2,...       关键词（默认: vpn,password,passwd,pwd,account,账户,密码）\n"
                    L"  -t N                 线程数（默认=逻辑核数）\n"
                    L"  -o 文件              输出到文件（UTF-8 CSV）\n"
                    L"  -norec               非递归\n"
                    L"  -followsymlink       跟随符号链接/重解析点\n";
        return 1;
    }
    if (cfg.scan_all) {
        DWORD mask = GetLogicalDrives();
        for (int i = 0; i < 26; ++i) if (mask & (1u << i)) { wchar_t root[4] = { (wchar_t)(L'A' + i), L':', L'\\', 0 }; UINT type = GetDriveTypeW(root); if (type == DRIVE_FIXED || type == DRIVE_REMOVABLE) paths.push_back(wstring(root)); }
    }
    for (auto &p : paths) p = normalize_path(p);
    DirQueue dq; for (auto &p : paths) dq.push(p);
    init_output(cfg);
    std::cout << "Scanning started with " << cfg.threads << " threads...\n";
    run_workers(dq, cfg);
    std::wcout << L"Output file: " << get_output_path() << L"\n";
    std::cout << "Total matches: " << g_match_count.load(std::memory_order_relaxed) << "\n";
    std::cout << "Keywords: " << join_keywords_utf8(cfg.keywords) << "\n";
    return 0;
}


