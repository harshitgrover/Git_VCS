#pragma once
#include <string>
#include <vector>
#include <map>

using namespace std;

namespace minigit {


class Index {
public:
    Index(const string& gitdir);
    
    void add(const string& path, const string& hash);
    void remove(const string& path);
    void read();
    void write() const;
    void clear();
    
    const map<string, string>& getEntries() const { return entries_; }

private:
    string index_path_;
    map<string, string> entries_;
};

} // namespace minigit
