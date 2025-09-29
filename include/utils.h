#pragma once
#include <string>

std::wstring to_lower_w(const std::wstring &s);
std::wstring join_path(const std::wstring &a, const std::wstring &b);
std::string utf16_to_utf8(const std::wstring &w);
std::wstring make_timestamped_filename();
std::string join_keywords_utf8(const std::vector<std::wstring> &kws);
int get_default_threads();
std::wstring normalize_path(const std::wstring &p);

// Recycle Bin helpers
// Returns true if the given absolute or normalized path is under a $Recycle.Bin directory
bool path_is_under_recycle_bin(const std::wstring &path);
// Try to parse a Recycle Bin metadata file ($Ixxxxxx) to extract the original full path and file name
bool try_parse_recycle_i_file(const std::wstring &i_file_path,
                              std::wstring &original_fullpath,
                              std::wstring &original_name);

// Filesystem helpers
bool is_path_on_ntfs(const std::wstring &rootPath);


