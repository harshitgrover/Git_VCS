#include "Index.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

namespace minigit {

Index::Index(const string& gitdir) {
    index_path_ = (fs::path(gitdir) / "index").string();
    read();
}

void Index::add(const string& path, const string& hash) {
    entries_[path] = hash;
}

void Index::remove(const string& path) {
    entries_.erase(path);
}

void Index::read() {
    entries_.clear();
    ifstream in(index_path_);
    if (!in.is_open()) return;
    
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        size_t space_idx = line.find(' ');
        if (space_idx != string::npos) {
            string hash = line.substr(0, space_idx);
            string path = line.substr(space_idx + 1);
            entries_[path] = hash;
        }
    }
}

void Index::write() const {
    ofstream out(index_path_, ios::trunc);
    for (const auto& [path, hash] : entries_) {
        out << hash << " " << path << "\n";
    }
}

void Index::clear() {
    entries_.clear();
}

} // namespace minigit
