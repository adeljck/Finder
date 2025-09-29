#include <string>
#include <vector>
#include <locale>
#include "../include/config.h"
#include "../include/utils.h"

using std::wstring; using std::vector;

Config parse_args(int argc, wchar_t **argv, vector<wstring> &paths) {
    Config cfg;
    cfg.threads = get_default_threads();
    for (int i = 1; i < argc; ++i) {
        wstring s = argv[i];
        if (s == L"-t" && i + 1 < argc) cfg.threads = std::stoi(argv[++i]);
        else if (s == L"-o" && i + 1 < argc) cfg.output_file = argv[++i];
        else if (s == L"-norec") cfg.recursive = false;
        else if (s == L"-followsymlink") cfg.follow_symlink = true;
        else if (s == L"-all") cfg.scan_all = true;
        else if (s == L"--debug") cfg.debug = true;
        else if (s == L"--debug-denied") cfg.debug_denied = true;
        else if (s == L"-k" && i + 1 < argc) {
            wstring k = argv[++i]; size_t pos = 0;
            while (pos < k.size()) { size_t comma = k.find(L',', pos); wstring part = (comma == wstring::npos) ? k.substr(pos) : k.substr(pos, comma - pos); if (!part.empty()) cfg.keywords.push_back(to_lower_w(part)); if (comma == wstring::npos) break; pos = comma + 1; }
        } else paths.push_back(s);
    }
    if (cfg.keywords.empty()) cfg.keywords = { to_lower_w(L"vpn"), to_lower_w(L"password"), to_lower_w(L"passwd"), to_lower_w(L"pwd"), to_lower_w(L"account"), to_lower_w(L"账户"), to_lower_w(L"密码") };
    return cfg;
}


