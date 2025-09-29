#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <windows.h>
#include <cwctype>
#include <locale>
#include <string>
#include <vector>
#include "../include/utils.h"

using std::wstring; using std::string; using std::vector;

wstring to_lower_w(const wstring &s) {
    wstring out; out.resize(s.size());
    for (size_t i = 0; i < s.size(); ++i) out[i] = std::towlower(s[i]);
    return out;
}

wstring join_path(const wstring &a, const wstring &b) {
    if (a.empty()) return b;
    if (a.back() == L'\\' || a.back() == L'/') return a + b;
    return a + L"\\" + b;
}

string utf16_to_utf8(const wstring &w) {
    if (w.empty()) return {};
    int required = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), NULL, 0, NULL, NULL);
    if (required <= 0) return {};
    string out; out.resize(required);
    int written = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), out.data(), required, NULL, NULL);
    if (written <= 0) return {};
    return out;
}

wstring make_timestamped_filename() {
    SYSTEMTIME st{}; GetLocalTime(&st);
    wchar_t buf[64]{};
    swprintf(buf, 64, L"result_%04u%02u%02u_%02u%02u%02u.csv",
        (unsigned)st.wYear, (unsigned)st.wMonth, (unsigned)st.wDay,
        (unsigned)st.wHour, (unsigned)st.wMinute, (unsigned)st.wSecond);
    return wstring(buf);
}

string join_keywords_utf8(const vector<wstring> &kws) {
    string out;
    for (size_t i = 0; i < kws.size(); ++i) { if (i) out += ","; out += utf16_to_utf8(kws[i]); }
    return out;
}

int get_default_threads() {
    SYSTEM_INFO si{}; GetSystemInfo(&si);
    int n = (int)si.dwNumberOfProcessors;
    return n > 0 ? n : 1;
}

wstring normalize_path(const wstring &p) { if (p.empty()) return p; wstring out = p; if (out.size() == 3 && out[1] == L':' && (out[2] == L'\\' || out[2] == '/')) return out; while (!out.empty() && (out.back() == L'\\' || out.back() == '/')) out.pop_back(); return out; }


