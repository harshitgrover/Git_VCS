#include "Commit.hpp"
#include <ctime>

using namespace std;

namespace minigit {

Commit::Commit(const string& tree_hash, const vector<string>& parents, const string& message)
    : tree_hash_(tree_hash), parents_(parents), message_(message) {}

string Commit::type() const {
    return "commit";
}

string Commit::serialize() const {
    string content = "tree " + tree_hash_ + "\n";
    for (const auto& p : parents_) {
        if (!p.empty()) {
            content += "parent " + p + "\n";
        }
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
