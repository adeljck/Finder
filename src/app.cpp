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

static void print_usage() {
    std::wcout << L"Usage: finder.exe [options] [PATH1 PATH2 ...]\n"
                L"  -all                 Scan all available drives\n"
                L"  -k kw1,kw2,...       Keywords (default: vpn,password,passwd,pwd,account,账户,密码,账号,台账,服务器)\n"
                L"  -t N                 Number of threads (default = logical CPUs)\n"
                L"  -o FILE              Write results to FILE (UTF-8 CSV)\n"
                L"  -norec               Non-recursive\n"
                L"  -followsymlink       Follow symlinks/reparse points\n"
                L"  -nonntfs             Allow scanning non-NTFS volumes (skipped by default)\n"
                L"  -extexclude ext1,ext2  Exclude file extensions (comma-separated, no dots)\n"
                L"  -utf8bom             Write UTF-8 BOM at file start (for legacy tools)\n"
                L"  -h, --help           Show this help and exit\n";
}

int run_app(int argc, wchar_t **argv) {
    std::locale::global(std::locale(""));
    std::vector<wstring> paths;
    Config cfg = parse_args(argc, argv, paths);
    if (cfg.show_help) {
        print_usage();
        return 0;
    }
    if (cfg.invalid_args) {
        print_usage();
        return 2;
    }
    if (paths.empty() && !cfg.scan_all) {
        print_usage();
        return 1;
    }
    if (cfg.scan_all) {
        DWORD mask = GetLogicalDrives();
        for (int i = 0; i < 26; ++i) if (mask & (1u << i)) {
            wchar_t root[4] = { (wchar_t)(L'A' + i), L':', L'\\', 0 };
            UINT type = GetDriveTypeW(root);
            if (type == DRIVE_FIXED || type == DRIVE_REMOVABLE) {
                // Skip non-NTFS unless allowed
                if (!cfg.allow_non_ntfs && !is_path_on_ntfs(root)) continue;
                paths.push_back(wstring(root));
            }
        }
    }
    // Filter non-NTFS for explicitly provided paths if they look like roots
    {
        std::vector<wstring> filtered;
        filtered.reserve(paths.size());
        for (auto &p : paths) {
            wstring np = normalize_path(p);
            if (!cfg.allow_non_ntfs && np.size() == 3 && np[1] == L':' && (np[2] == L'\\' || np[2] == L'/')) {
                if (!is_path_on_ntfs(np)) continue;
            }
            filtered.push_back(np);
        }
        paths.swap(filtered);
    }
    DirQueue dq; for (auto &p : paths) dq.push(p);
    init_output(cfg);
    std::cout << "Scanning started with " << cfg.threads << " threads...\n";
    run_workers(dq, cfg);
    std::wcout << L"Output file: " << get_output_path() << L"\n";
    std::cout << "Total matches: " << g_match_count.load(std::memory_order_relaxed) << "\n";
    std::cout << "Keywords: " << join_keywords_utf8(cfg.keywords) << "\n";
    return 0;
}


