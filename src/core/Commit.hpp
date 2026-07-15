#pragma once
#include "GitObject.hpp"
#include <string>
#include <vector>

using namespace std;

namespace minigit {

class Commit : public GitObject {
public:
    Commit(const string& tree_hash, const vector<string>& parents, const string& message);

    string serialize() const override;
    string type() const override;

private:
    string tree_hash_;
    vector<string> parents_;
    string message_;
};

} // namespace minigit
