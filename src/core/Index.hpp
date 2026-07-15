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
    void read();
    void write() const;
    
    const map<string, string>& getEntries() const { return entries_; }

private:
    string index_path_;
    map<string, string> entries_;
};

} // namespace minigit
