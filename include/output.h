#pragma once
#include <string>
#include <mutex>
#include <atomic>
#include "config.h"

void output_match(const Config &cfg, const std::wstring &fullpath, const std::wstring &filename, const std::wstring &keyword);
extern std::mutex g_output_mutex;
extern std::atomic<unsigned long long> g_match_count;

// Initialize output target and open file early; safe to call once at startup
void init_output(const Config &cfg);
// Query the chosen output file path (empty if not initialized)
const std::wstring &get_output_path();


