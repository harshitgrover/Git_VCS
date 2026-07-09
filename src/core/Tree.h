#pragma once
#include "GitObject.h"
#include <string>
#include <vector>

using namespace std;

namespace minigit {

struct TreeEntry {
    string mode;
    string name;
    string hash;
};

class Tree : public GitObject {
public:
    Tree(const vector<TreeEntry>& entries);

    string serialize() const override;
    string type() const override;

private:
    vector<TreeEntry> entries_;
};

} // namespace minigit
