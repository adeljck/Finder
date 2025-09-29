#include <fstream>
#include <iostream>
#include <mutex>
#include <atomic>
#include <windows.h>
#include "../include/output.h"
#include "../include/utils.h"

std::mutex g_output_mutex;
std::atomic<unsigned long long> g_match_count{0};

static std::wofstream::traits_type::char_type;
static std::ofstream g_fout;
static std::wstring g_output_path;

void init_output(const Config &cfg) {
    std::lock_guard<std::mutex> lk(g_output_mutex);
    if (!g_output_path.empty()) return;
    g_output_path = cfg.output_file.empty() ? make_timestamped_filename() : cfg.output_file;
    g_fout.open(g_output_path, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!g_fout.is_open()) std::cerr << "Failed to open output file.\n";
    else if (cfg.utf8_bom) {
        static const unsigned char bom[3] = { 0xEF, 0xBB, 0xBF };
        g_fout.write(reinterpret_cast<const char*>(bom), 3);
    }
}

const std::wstring &get_output_path() { return g_output_path; }

void output_match(const Config &cfg, const std::wstring &fullpath, const std::wstring &filename, const std::wstring &keyword) {
    std::lock_guard<std::mutex> lk(g_output_mutex);
    if (g_output_path.empty()) init_output(cfg);

    std::string s_full = utf16_to_utf8(fullpath);
    std::string s_name = utf16_to_utf8(filename);
    std::string s_key = utf16_to_utf8(keyword);

    auto escape_csv = [](const std::string &s) {
        std::string r = "\"";
        for (char c : s) { if (c == '"') r += '"'; r += c; }
        r += '"';
        return r;
    };

    std::string line = escape_csv(s_full) + "," + escape_csv(s_name) + "," + escape_csv(s_key) + "\n";
    if (g_fout.is_open()) g_fout << line;
    if (cfg.debug) {
        // Print as wide to console to avoid mojibake on legacy consoles
        auto escape_csv_w = [](const std::wstring &ws) {
            std::wstring r = L"\"";
            for (wchar_t c : ws) { if (c == L'\"') r += L'\"'; r += c; }
            r += L'\"';
            return r;
        };
        std::wstring wline = escape_csv_w(fullpath) + L"," + escape_csv_w(filename) + L"," + escape_csv_w(keyword) + L"\r\n";
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut && hOut != INVALID_HANDLE_VALUE) {
            DWORD written = 0;
            WriteConsoleW(hOut, wline.c_str(), (DWORD)wline.size(), &written, NULL);
        } else {
            // Fallback
            std::wcout << wline;
        }
    }
    g_match_count.fetch_add(1ULL, std::memory_order_relaxed);
}


