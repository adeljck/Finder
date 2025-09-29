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

static bool wcs_ieq_prefix(const wstring &a, const wstring &prefix) {
    if (a.size() < prefix.size()) return false;
    for (size_t i = 0; i < prefix.size(); ++i) {
        wchar_t ca = std::towlower(a[i]);
        wchar_t cb = std::towlower(prefix[i]);
        if (ca != cb) return false;
    }
    return true;
}

bool path_is_under_recycle_bin(const wstring &path) {
    // Common patterns: C:\$Recycle.Bin\<SID>\..., older: C:\RECYCLER\..., C:\RECYCLED\...
    // Accept case-insensitively
    if (path.size() < 4) return false;
    // Drive root based check
    // Build three candidate prefixes
    wstring driveRoot = path.substr(0, 3); // e.g. C:\\
    if (driveRoot.size() != 3 || driveRoot[1] != L':' ) return false;
    wstring p1 = driveRoot + L"$Recycle.Bin\\";
    wstring p2 = driveRoot + L"RECYCLER\\";
    wstring p3 = driveRoot + L"RECYCLED\\";
    return wcs_ieq_prefix(path, p1) || wcs_ieq_prefix(path, p2) || wcs_ieq_prefix(path, p3);
}

// Try to parse Windows Vista+ $I file structure (for $I + same stem as $R)
// Layout (Vista/Win7+):
// 0x00: 8 bytes: header/version (value 0x01)
// 0x08: 8 bytes: original file size (little endian)
// 0x10: 8 bytes: deletion time (FILETIME)
// 0x18: UTF-16LE null-terminated original path string
bool try_parse_recycle_i_file(const wstring &i_file_path,
                              wstring &original_fullpath,
                              wstring &original_name) {
    HANDLE h = CreateFileW(i_file_path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return false;
    bool ok = false;
    DWORD sizeLow = GetFileSize(h, NULL);
    if (sizeLow == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) { CloseHandle(h); return false; }
    const DWORD kMinSize = 0x20; // header + minimal path
    if (sizeLow < kMinSize) { CloseHandle(h); return false; }
    // Map small header first
    BYTE header[0x20]; DWORD read = 0;
    if (!ReadFile(h, header, sizeof(header), &read, NULL) || read < sizeof(header)) { CloseHandle(h); return false; }
    // Version at offset 0 expected to be 0x01 (8-byte little-endian)
    unsigned long long version = 0;
    for (int i = 7; i >= 0; --i) { version = (version << 8) | header[i]; }
    if (version != 0x01ULL) { /* some older formats may differ */ }
    // Read the rest as UTF-16LE string
    // Move file pointer to 0x18
    LARGE_INTEGER li{}; li.QuadPart = 0x18; SetFilePointerEx(h, li, NULL, FILE_BEGIN);
    // Remaining bytes count
    DWORD remain = sizeLow >= 0x18 ? (sizeLow - 0x18) : 0;
    if (remain < 2) { CloseHandle(h); return false; }
    vector<wchar_t> buf; buf.resize((remain / 2) + 1);
    read = 0;
    if (!ReadFile(h, buf.data(), remain, &read, NULL) || read == 0) { CloseHandle(h); return false; }
    size_t wlen = read / 2;
    buf[wlen] = L'\0';
    original_fullpath.assign(buf.data());
    // Extract name from path
    size_t pos = original_fullpath.find_last_of(L"\\/");
    original_name = (pos == wstring::npos) ? original_fullpath : original_fullpath.substr(pos + 1);
    ok = !original_fullpath.empty();
    CloseHandle(h);
    return ok;
}

bool is_path_on_ntfs(const wstring &rootPath) {
    // Expect a root like X:\
    if (rootPath.size() < 3 || rootPath[1] != L':') return false;
    wchar_t fsName[32]{};
    if (!GetVolumeInformationW(rootPath.c_str(), NULL, 0, NULL, NULL, NULL, fsName, 32)) return false;
    // Case-insensitive compare to NTFS
    for (wchar_t *p = fsName; *p; ++p) *p = std::towlower(*p);
    return wcscmp(fsName, L"ntfs") == 0;
}


