#pragma once
#include "GitObject.h"
#include <string>

using namespace std;

namespace minigit {

class Commit : public GitObject {
public:
    Commit(const string& tree_hash, const string& parent_hash, const string& message);

    string serialize() const override;
    string type() const override;

private:
    string tree_hash_;
    string parent_hash_;
    string message_;
};

} // namespace minigit
