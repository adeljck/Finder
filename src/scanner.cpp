#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include "../include/scanner.h"
#include "../include/utils.h"
#include "../include/output.h"

using std::wstring; using std::vector; using std::thread;

void DirQueue::push(wstring p) { std::unique_lock<std::mutex> lk(m); q.push(std::move(p)); ++pending; cv.notify_one(); }
bool DirQueue::pop(wstring &out) { std::unique_lock<std::mutex> lk(m); while (q.empty() && !stop) cv.wait(lk); if (q.empty()) return false; out = std::move(q.front()); q.pop(); return true; }
void DirQueue::task_done() { std::unique_lock<std::mutex> lk(m); if (pending > 0) --pending; if (pending == 0) { stop = true; cv.notify_all(); } }

static bool name_contains_keyword(const wstring &name_lower, const Config &cfg, wstring &found) {
    for (auto &k : cfg.keywords) if (name_lower.find(k) != wstring::npos) { found = k; return true; }
    return false;
}

static void worker(DirQueue &dq, const Config &cfg) {
    wstring dir;
    while (dq.pop(dir)) {
        wstring search = join_path(dir, L"*");
        WIN32_FIND_DATAW fd;
        HANDLE h = FindFirstFileW(search.c_str(), &fd);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) continue;
                wstring name = fd.cFileName;
                wstring name_lower = to_lower_w(name);
                wstring found_keyword;
                bool matched = false;
                wstring out_name = name;
                wstring out_full = join_path(dir, name);
                bool is_dir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                // Extension exclude filter (files only)
                if (!is_dir && !cfg.exclude_exts.empty()) {
                    size_t dot = name.find_last_of(L'.');
                    if (dot != wstring::npos && dot + 1 < name.size()) {
                        wstring ext = to_lower_w(name.substr(dot + 1));
                        bool excluded = false;
                        for (auto &ex : cfg.exclude_exts) { if (ext == ex) { excluded = true; break; } }
                        if (excluded) { continue; }
                    }
                }
                // Recycle Bin: if this is an $Ixxxxx file, try original name match
                if (!matched && path_is_under_recycle_bin(dir)) {
                    // $Ixxxxx.* files carry metadata
                    if (!name.empty() && name[0] == L'$') {
                        if (name.size() >= 2 && (name[1] == L'I' || name[1] == L'i')) {
                            wstring orig_full, orig_name;
                            if (try_parse_recycle_i_file(out_full, orig_full, orig_name)) {
                                wstring orig_lower = to_lower_w(orig_name);
                                if (name_contains_keyword(orig_lower, cfg, found_keyword)) {
                                    matched = true;
                                    out_name = orig_name;
                                    out_full = orig_full;
                                }
                            }
                        }
                    }
                }
                if (!matched) {
                    if (name_contains_keyword(name_lower, cfg, found_keyword)) {
                        matched = true;
                    }
                }
                if (matched) output_match(cfg, out_full, out_name, found_keyword);
                bool is_reparse = (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
                if (is_dir && cfg.recursive) { if (!(is_reparse && !cfg.follow_symlink)) dq.push(join_path(dir, name)); }
            } while (FindNextFileW(h, &fd));
            FindClose(h);
        } else {
            if (GetLastError() == ERROR_ACCESS_DENIED && cfg.debug_denied) {
                std::wcerr << L"DENIED: " << dir << L"\n";
            }
        }
        dq.task_done();
    }
}

void run_workers(DirQueue &dq, const Config &cfg) {
    vector<thread> workers; workers.reserve((size_t)cfg.threads);
    for (int i = 0; i < cfg.threads; ++i) workers.emplace_back(worker, std::ref(dq), std::ref(cfg));
    for (auto &t : workers) if (t.joinable()) t.join();
}


