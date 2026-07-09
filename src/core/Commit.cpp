#include "Commit.h"
#include <ctime>

using namespace std;

namespace minigit {

Commit::Commit(const string& tree_hash, const string& parent_hash, const string& message)
    : tree_hash_(tree_hash), parent_hash_(parent_hash), message_(message) {}

string Commit::type() const {
    return "commit";
}

string Commit::serialize() const {
    string content = "tree " + tree_hash_ + "\n";
    if (!parent_hash_.empty()) {
        content += "parent " + parent_hash_ + "\n";
    }
    
    time_t now = time(nullptr);
    string time_str = to_string(now);
    
    content += "author minigit <minigit@example.com> " + time_str + " +0000\n";
    content += "committer minigit <minigit@example.com> " + time_str + " +0000\n\n";
    content += message_ + "\n";
    
    string header = type() + " " + to_string(content.size()) + '\0';
    return header + content;
}

} // namespace minigit
