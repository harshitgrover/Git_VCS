#include "GcCommand.hpp"
#include "../core/Repository.hpp"
#include "../core/Utils.hpp"
#include <iostream>
#include <filesystem>
#include <sstream>
#include <fstream>

using namespace std;
namespace fs = std::filesystem;

namespace minigit {

void GcCommand::execute(const vector<string>& args) {
    Repository repo(".");
    string gitdir = repo.getGitDir();
    
    set<string> reachable;
    
    fs::path heads_dir = fs::path(gitdir) / "refs" / "heads";
    if (fs::exists(heads_dir)) {
        for (const auto& entry : fs::recursive_directory_iterator(heads_dir)) {
            if (entry.is_regular_file()) {
                ifstream in(entry.path());
                string hash;
                if (getline(in, hash)) {
                    mark_commit(gitdir, hash, reachable);
                }
            }
        }
    }
    
    fs::path head_path = fs::path(gitdir) / "HEAD";
    if (fs::exists(head_path)) {
        ifstream in(head_path);
        string line;
        if (getline(in, line)) {
            if (line.find("ref: ") != 0) {
                mark_commit(gitdir, line, reachable);
            }
        }
    }
    
    fs::path index_path = fs::path(gitdir) / "index";
    if (fs::exists(index_path)) {
        ifstream in(index_path);
        string line;
        while (getline(in, line)) {
            if (line.empty()) continue;
            size_t space_idx = line.find(' ');
            if (space_idx != string::npos) {
                string hash = line.substr(0, space_idx);
                mark_blob(gitdir, hash, reachable);
            }
        }
    }
    
    int deleted = 0;
    fs::path obj_dir = fs::path(gitdir) / "objects";
    if (fs::exists(obj_dir)) {
        for (const auto& dir_entry : fs::directory_iterator(obj_dir)) {
            if (dir_entry.is_directory()) {
                string dir_name = dir_entry.path().filename().string();
                for (const auto& file_entry : fs::directory_iterator(dir_entry.path())) {
                    string file_name = file_entry.path().filename().string();
                    string hash = dir_name + file_name;
                    
                    if (reachable.find(hash) == reachable.end()) {
                        fs::remove(file_entry.path());
                        deleted++;
                    }
                }
            }
        }
    }
    
    cout << "Garbage collection complete. Deleted " << deleted << " orphaned objects.\n";
}

void GcCommand::mark_commit(const string& gitdir, const string& hash, set<string>& reachable) {
    if (reachable.count(hash)) return;
    reachable.insert(hash);
    
    string content = utils::read_object_content(gitdir, hash);
    if (content.empty()) return;
    
    size_t null_idx = content.find('\0');
    if (null_idx != string::npos) {
        string body = content.substr(null_idx + 1);
        istringstream iss(body);
        string line;
        while (getline(iss, line)) {
            if (line.empty()) break;
            if (line.substr(0, 5) == "tree ") {
                mark_tree(gitdir, line.substr(5), reachable);
            } else if (line.substr(0, 7) == "parent ") {
                mark_commit(gitdir, line.substr(7), reachable);
            }
        }
    }
}

void GcCommand::mark_tree(const string& gitdir, const string& hash, set<string>& reachable) {
    if (reachable.count(hash)) return;
    reachable.insert(hash);
    
    string content = utils::read_object_content(gitdir, hash);
    if (content.empty()) return;
    
    size_t null_idx = content.find('\0');
    if (null_idx != string::npos) {
        string body = content.substr(null_idx + 1);
        size_t pos = 0;
        while (pos < body.size()) {
            size_t mode_space = body.find(' ', pos);
            if (mode_space == string::npos) break;
            size_t name_null = body.find('\0', mode_space + 1);
            if (name_null == string::npos) break;
            
            if (name_null + 1 + 64 <= body.size()) {
                string blob_hash = body.substr(name_null + 1, 64);
                mark_blob(gitdir, blob_hash, reachable);
                pos = name_null + 1 + 64;
            } else {
                break;
            }
        }
    }
}

void GcCommand::mark_blob(const string& gitdir, const string& hash, set<string>& reachable) {
    if (reachable.count(hash)) return;
    reachable.insert(hash);
    
    string content = utils::read_object_content(gitdir, hash);
    if (content.empty()) return;
    
    size_t null_idx = content.find('\0');
    if (null_idx != string::npos) {
        string body = content.substr(null_idx + 1);
        if (body.substr(0, 6) == "chunk ") {
            istringstream iss(body);
            string line;
            while (getline(iss, line)) {
                if (line.substr(0, 6) == "chunk ") {
                    string chunk_hash = line.substr(6);
                    reachable.insert(chunk_hash);
                }
            }
        }
    }
}

} // namespace minigit
