#pragma once
#include <string>
#include <vector>

struct Config {
    std::vector<std::wstring> keywords;
    int threads = 1;
    bool recursive = true;
    bool follow_symlink = false;
    std::wstring output_file;
    bool scan_all = false;
    bool debug = false;
    bool debug_denied = false;
    bool allow_non_ntfs = false; // if false, non-NTFS volumes are skipped
    bool show_help = false;
    bool invalid_args = false;
};

Config parse_args(int argc, wchar_t **argv, std::vector<std::wstring> &paths);


