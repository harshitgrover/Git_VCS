#include "Tree.h"
#include <sstream>

using namespace std;

namespace minigit {

Tree::Tree(const vector<TreeEntry>& entries) : entries_(entries) {}

string Tree::type() const {
    return "tree";
}

string Tree::serialize() const {
    string content;
    for (const auto& entry : entries_) {
        content += entry.mode + " " + entry.name + '\0' + entry.hash;
    }
    
    string header = type() + " " + to_string(content.size()) + '\0';
    return header + content;
}

} // namespace minigit
