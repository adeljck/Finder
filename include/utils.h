#pragma once
#include <string>

std::wstring to_lower_w(const std::wstring &s);
std::wstring join_path(const std::wstring &a, const std::wstring &b);
std::string utf16_to_utf8(const std::wstring &w);
std::wstring make_timestamped_filename();
std::string join_keywords_utf8(const std::vector<std::wstring> &kws);
int get_default_threads();
std::wstring normalize_path(const std::wstring &p);


